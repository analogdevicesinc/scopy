#include "SnapshotSource.h"

#include "AcquisitionEngine.h"
#include "DataStore.h"
#include "SnapshotSourceWidget.h"

#include <QMutexLocker>
#include <QRegularExpression>
#include <QWidget>

namespace scopy {
namespace acq {

SnapshotSource::SnapshotSource(const QString &id, QObject *parent)
	: SourceBlock(id, parent)
{}

void SnapshotSource::setSourceStore(DataStore *store)
{
	QMutexLocker lk(&m_mutex);
	m_srcStore = store;
}

QString SnapshotSource::sanitizeTitle(const QString &title)
{
	// Underscore is the key separator, so a title carrying one would reparse as two segments
	// and steal the stage. Whitespace goes the same way.
	static const QRegularExpression breaks(QStringLiteral("[_\\s]+"));
	return title.trimmed().replace(breaks, QStringLiteral("-"));
}

DataKey SnapshotSource::keyFor(const QString &sourceId, const QString &title)
{
	if(title.isEmpty()) {
		return DataKey();
	}
	return DataKey::withStage(sourceId, title, QStringLiteral("snapshot"));
}

void SnapshotSource::dropKey(DataStore *store, const QString &title) const
{
	if(store && !title.isEmpty()) {
		store->remove(keyFor(name(), title));
	}
}

// --- slot management ---------------------------------------------------------

int SnapshotSource::addSlot()
{
	int index;
	{
		QMutexLocker lk(&m_mutex);
		m_slots.append(Slot());
		index = m_slots.size() - 1;
	}
	Q_EMIT slotsChanged();
	return index;
}

void SnapshotSource::removeSlot(int index)
{
	DataStore *store = nullptr;
	QString title;
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_slots.size()) {
			return;
		}
		title = m_slots.takeAt(index).title;
		store = m_srcStore;
	}
	dropKey(store, title);
	Q_EMIT slotsChanged();
}

QList<SnapshotSource::Slot> SnapshotSource::allSlots() const
{
	QMutexLocker lk(&m_mutex);
	return m_slots;
}

std::optional<SnapshotSource::Slot> SnapshotSource::slotAt(int index) const
{
	QMutexLocker lk(&m_mutex);
	if(index < 0 || index >= m_slots.size()) {
		return std::nullopt;
	}
	return m_slots.at(index);
}

void SnapshotSource::setSlotSource(int index, const DataKey &key)
{
	QMutexLocker lk(&m_mutex);
	if(index < 0 || index >= m_slots.size()) {
		return;
	}
	// Deliberately does not invalidate the capture: retargeting says what the *next* capture
	// reads, and dropping frozen data on a mis-click would be destructive.
	m_slots[index].source = key;
}

void SnapshotSource::setSlotCyclic(int index, bool on)
{
	DataStore *store = nullptr;
	QString title;
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_slots.size()) {
			return;
		}
		m_slots[index].cyclic = on;
		m_slots[index].playhead = 0;
		title = m_slots.at(index).title;
		store = m_srcStore;
	}
	// The two modes publish differently shaped buffers, so what the previous one left has to
	// go — playback chunks pushed onto a copied-out history read as a growing tail.
	dropKey(store, title);
	Q_EMIT slotCaptured(index);
}

bool SnapshotSource::isTitleAvailable(const QString &title, int exceptIndex) const
{
	const QString clean = sanitizeTitle(title);
	if(clean.isEmpty()) {
		return false;
	}
	QMutexLocker lk(&m_mutex);
	for(int i = 0; i < m_slots.size(); ++i) {
		if(i != exceptIndex && m_slots.at(i).title == clean) {
			return false;
		}
	}
	return true;
}

bool SnapshotSource::setSlotTitle(int index, const QString &title)
{
	const QString clean = sanitizeTitle(title);
	DataStore *store = nullptr;
	QString old;
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_slots.size() || clean.isEmpty()) {
			return false;
		}
		if(m_slots.at(index).title == clean) {
			return true; // the reader committed the name they already had
		}
		for(int i = 0; i < m_slots.size(); ++i) {
			if(i != index && m_slots.at(i).title == clean) {
				return false;
			}
		}
		old = m_slots.at(index).title;
		m_slots[index].title = clean;
		store = m_srcStore;
	}
	// Otherwise the pre-rename key stays in the store and every picker keeps offering a stream
	// nothing writes to.
	dropKey(store, old);
	Q_EMIT slotsChanged();
	return true;
}

// --- capture -----------------------------------------------------------------

void SnapshotSource::capture(int index)
{
	DataStore *store = nullptr;
	Slot s;
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_slots.size()) {
			return;
		}
		s = m_slots.at(index);
		store = m_srcStore;
	}
	if(!store) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("capture ignored: no DataStore was injected"));
		return;
	}
	if(s.title.isEmpty()) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("snapshot %1: capture ignored, the snapshot has no name").arg(index));
		return;
	}
	if(s.source.key.isEmpty()) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("%1: capture ignored, no source selected").arg(s.title));
		return;
	}

	// The whole capture: one buffer copy. Done with m_mutex released — DataStore takes its
	// own, and streamInfo() below re-enters this block.
	s.data = store->snapshot(s.source);
	if(s.data.empty()) {
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("%1: %2 has no samples to capture").arg(s.title, s.source.toString()));
		return;
	}

	s.unit = QString();
	s.sampleRate = 0.0;
	if(auto *engine = qobject_cast<AcquisitionEngine *>(parent())) {
		if(const std::optional<StreamInfo> info = engine->streamInfo(s.source)) {
			s.unit = info->unit;
			s.sampleRate = info->sampleRate;
		}
	}
	s.annInfo = store->annotationInfo(s.source);
	s.captured = true;
	s.capturedFrom = s.source.toString();
	s.playhead = 0;

	{
		QMutexLocker lk(&m_mutex);
		// Re-checked: the index was captured before the lock was released.
		if(index < 0 || index >= m_slots.size()) {
			return;
		}
		m_slots[index] = s;
	}
	// A re-capture replaces the recording, so the previous one's chunks must not be left for
	// playback to push onto.
	dropKey(store, s.title);
	Q_EMIT slotCaptured(index);
}

void SnapshotSource::clearSlot(int index)
{
	DataStore *store = nullptr;
	QString title;
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_slots.size()) {
			return;
		}
		Slot &s = m_slots[index];
		title = s.title;
		store = m_srcStore;
		// Back to an uncaptured slot, keeping what the reader configured.
		Slot fresh;
		fresh.title = s.title;
		fresh.source = s.source;
		fresh.cyclic = s.cyclic;
		s = fresh;
	}
	// acquire() stops publishing, but the store would keep what it already holds.
	dropKey(store, title);
	Q_EMIT slotCaptured(index);
}

// --- the pipeline side -------------------------------------------------------

void SnapshotSource::onStart()
{
	SourceBlock::onStart();
	QMutexLocker lk(&m_mutex);
	for(Slot &s : m_slots) {
		s.playhead = 0;
	}
}

void SnapshotSource::acquire(DataStore *store)
{
	if(m_stopRequested) {
		return;
	}

	// No signal for the advancing position: the widget polls it. One emission per slot per
	// cycle would be queued GUI work at acquisition rate, for a counter nobody can read.
	QMutexLocker lk(&m_mutex);
	for(Slot &s : m_slots) {
		if(!s.captured || s.data.empty()) {
			continue;
		}

		const int frames = s.chunkCount();
		const DataKey key = keyFor(m_name, s.title);

		if(s.cyclic && frames > 1) {
			// write(), so the store applies the key's depth claims and consumers see a
			// stream arriving, exactly as from a live source.
			store->write(key, s.chunkAt(s.playhead));
			s.playhead = (s.playhead + 1) % frames;
		} else {
			// copy(), not write(): this is a whole history, and pushing it as a chunk
			// would flatten it and grow the stream every cycle.
			store->copy(key, s.data);
		}
		if(s.annInfo) {
			store->setAnnotationInfo(key, *s.annInfo);
		}
	}
}

QList<DataKey> SnapshotSource::outputKeys() const
{
	QMutexLocker lk(&m_mutex);
	QList<DataKey> keys;
	keys.reserve(m_slots.size());
	for(const Slot &s : m_slots) {
		// Only what acquire() publishes: an uncaptured slot would put a stream in every
		// picker that never gets a sample.
		if(s.captured && !s.title.isEmpty()) {
			keys.append(keyFor(m_name, s.title));
		}
	}
	return keys;
}

std::optional<StreamInfo> SnapshotSource::streamInfo(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	for(const Slot &s : m_slots) {
		if(s.title.isEmpty() || key != keyFor(m_name, s.title)) {
			continue;
		}
		StreamInfo info;
		info.sampleRate = s.sampleRate;
		info.unit = s.unit;
		info.label = s.capturedFrom.isEmpty() ? s.title
						      : QStringLiteral("%1 (%2)").arg(s.title, s.capturedFrom);
		info.kind = ReprKind::Curve;
		// No xKey — a frozen curve on a live abscissa would drift. colorIndex left at -1
		// so the view assigns one rather than reusing the captured trace's colour.
		return info;
	}
	return std::nullopt;
}

QWidget *SnapshotSource::createSettingsWidget(QWidget *parent)
{
	// Fallback with no store or engine, so the combos list nothing. A host that wants a usable
	// panel builds SnapshotSourceWidget itself and injects it with setSettingsWidget().
	return withBaseSettings(new SnapshotSourceWidget(this, nullptr, nullptr), parent);
}

} // namespace acq
} // namespace scopy

#include "moc_SnapshotSource.cpp"
