/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "filesourceblock.h"

#include "filesourcewidget.h"
#include "sourceregistry.h"

#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/SnapshotSource.h>

#include <gui/style.h>

#include <QFileInfo>
#include <QMutexLocker>

using namespace scopy;
using namespace scopy::adc;

int FileSourceBlock::Slot::sampleCount() const
{
	return (reader && !channels.isEmpty()) ? int(reader->samples(0).size()) : 0;
}

FileSourceBlock::FileSourceBlock(const QString &id, QObject *parent)
	: SourceBlock(id, parent)
	// The store to publish into outside a cycle. Taken from the engine parent rather than
	// injected, so a file loaded while the engine is stopped reaches views immediately.
	, m_store(engineStore())
{
	// The base settings widget's checkboxes toggle channels, and a toggle is exactly the kind
	// of state change that has to reach the store.
	connect(this, &SourceBlock::channelEnabledChanged, this, [this](const QString &, bool) { publish(); });
}

FileSourceBlock::~FileSourceBlock() = default;

FileSourceBlock::Slot *FileSourceBlock::slotAt(int index)
{
	if(index < 0 || index >= int(m_slots.size())) {
		return nullptr;
	}
	return m_slots[index].get();
}

const FileSourceBlock::Slot *FileSourceBlock::slotAt(int index) const
{
	if(index < 0 || index >= int(m_slots.size())) {
		return nullptr;
	}
	return m_slots[index].get();
}

int FileSourceBlock::addSlot()
{
	int index;
	{
		QMutexLocker lk(&m_mutex);
		m_slots.push_back(std::make_unique<Slot>());
		index = int(m_slots.size()) - 1;
	}
	Q_EMIT slotsChanged();
	return index;
}

void FileSourceBlock::removeSlot(int index)
{
	QStringList channels;
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot) {
			return;
		}
		channels = slot->channels;
		m_slots.erase(m_slots.begin() + index);
	}
	for(const QString &ch : channels) {
		removeChannel(ch);
	}
	dropSlotKeys(m_store, channels);
	Q_EMIT slotsChanged();
	Q_EMIT published();
}

QList<FileSourceBlock::SlotInfo> FileSourceBlock::slotInfos() const
{
	QMutexLocker lk(&m_mutex);
	QList<SlotInfo> out;
	out.reserve(int(m_slots.size()));
	for(const std::unique_ptr<Slot> &slot : m_slots) {
		SlotInfo info;
		info.title = slot->title;
		info.path = slot->path;
		info.channelCount = slot->channels.size();
		info.sampleCount = slot->sampleCount();
		info.sampleRate = slot->rate();
		info.declaresRate = slot->declaredRate > 0.0;
		info.cyclic = slot->cyclic;
		info.playhead = slot->cyclic ? slot->playhead : 0;
		if(auto *csv = dynamic_cast<CsvFileReader *>(slot->reader.get())) {
			info.firstColumnMode = csv->firstColumnMode();
		}
		out.append(info);
	}
	return out;
}

QString FileSourceBlock::uniqueTitle(const QString &path, int exceptIndex) const
{
	const QString base = acq::SnapshotSource::sanitizeTitle(QFileInfo(path).completeBaseName());
	const QString stem = base.isEmpty() ? QStringLiteral("file") : base;

	for(int n = 1;; ++n) {
		const QString candidate = n == 1 ? stem : QStringLiteral("%1-%2").arg(stem).arg(n);
		bool taken = false;
		for(int i = 0; i < int(m_slots.size()); ++i) {
			if(i != exceptIndex && m_slots[i]->title == candidate) {
				taken = true;
				break;
			}
		}
		if(!taken) {
			return candidate;
		}
	}
}

bool FileSourceBlock::setSlotFile(int index, const QString &path)
{
	std::unique_ptr<FileReader> reader = makeFileReader(path);
	if(!reader) {
		report(acq::AcquisitionError::Severity::Warning,
		       QStringLiteral("Unsupported file type: %1").arg(QFileInfo(path).fileName()));
		return false;
	}

	QString err;
	if(!reader->open(path, &err)) {
		report(acq::AcquisitionError::Severity::Warning, err);
		return false;
	}

	QStringList previous;
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot) {
			return false;
		}
		previous = slot->channels;
		slot->title = uniqueTitle(path, index);
		slot->reader = std::move(reader);
		slot->path = path;
		slot->playhead = 0;
		slot->userRate = 0.0;
	}

	// A new recording replaces the old one, and the title changed with it, so the previous
	// file's keys must not be there for the new one's chunks to land on.
	dropSlotKeys(m_store, previous);
	adoptChannels(index);

	report(acq::AcquisitionError::Severity::Info,
	       QStringLiteral("%1: %2 channels, %3 samples")
		       .arg(QFileInfo(path).fileName())
		       .arg(slotInfos().at(index).channelCount)
		       .arg(slotInfos().at(index).sampleCount));

	Q_EMIT slotChanged(index);
	publish();
	return true;
}

void FileSourceBlock::setSlotCyclic(int index, bool on)
{
	QStringList channels;
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot || slot->cyclic == on) {
			return;
		}
		slot->cyclic = on;
		slot->playhead = 0;
		channels = slot->channels;
	}
	// The two modes write the same keys in incompatible shapes — a whole history one way, a
	// growing stream of chunks the other — so what the old mode left has to go.
	dropSlotKeys(m_store, channels);
	Q_EMIT slotChanged(index);
	// Cyclic publishes from acquire() only: pushing the whole file here would be the very
	// history the mode exists to avoid.
	if(!on) {
		publish();
	} else {
		Q_EMIT published();
	}
}

void FileSourceBlock::setSlotSampleRate(int index, double sr)
{
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot || qFuzzyCompare(slot->userRate, sr)) {
			return;
		}
		slot->userRate = sr;
	}
	Q_EMIT slotChanged(index);
	publish();
}

void FileSourceBlock::setSlotFirstColumnMode(int index, CsvFileReader::FirstColumnMode mode)
{
	QStringList channels;
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot) {
			return;
		}
		auto *csv = dynamic_cast<CsvFileReader *>(slot->reader.get());
		if(!csv || csv->firstColumnMode() == mode) {
			return;
		}
		csv->setFirstColumnMode(mode);
		slot->playhead = 0;
		channels = slot->channels;
	}
	// The channels shift by one column, so every key's contents change meaning.
	dropSlotKeys(m_store, channels);
	adoptChannels(index);
	Q_EMIT slotChanged(index);
	publish();
}

void FileSourceBlock::adoptChannels(int index)
{
	QStringList previous;
	QStringList names;
	{
		QMutexLocker lk(&m_mutex);
		Slot *slot = slotAt(index);
		if(!slot || !slot->reader) {
			return;
		}
		previous = slot->channels;
		slot->declaredRate = slot->reader->sampleRate();
		slot->channels.clear();
		// The slot title prefixes the channel so two files' streams stay apart, and '_'
		// separates the DataKey components, so a name carrying one would steal the stage.
		for(const QString &raw : slot->reader->channels()) {
			slot->channels.append(
				QStringLiteral("%1-%2").arg(slot->title, acq::SnapshotSource::sanitizeTitle(raw)));
		}
		names = slot->channels;
	}

	for(const QString &ch : previous) {
		if(!names.contains(ch)) {
			if(m_store) {
				m_store->remove(acq::DataKey::raw(id(), ch));
			}
			removeChannel(ch);
		}
	}
	for(const QString &ch : names) {
		// Channels the previous file also had keep their enable; new ones start on.
		enableChannel(ch, previous.contains(ch) ? isChannelEnabled(ch) : true);
	}
}

void FileSourceBlock::dropSlotKeys(scopy::acq::DataStore *store, const QStringList &channels) const
{
	if(!store) {
		return;
	}
	// With m_mutex released, for the reason apply() gives.
	for(const QString &ch : channels) {
		store->remove(acq::DataKey::raw(id(), ch));
	}
}

void FileSourceBlock::collectWhole(const Slot &slot, Writes &writes, Keys &drops) const
{
	for(int i = 0; i < slot.channels.size(); ++i) {
		const acq::DataKey key = acq::DataKey::raw(id(), slot.channels.at(i));
		if(!isChannelEnabled(slot.channels.at(i))) {
			drops.append(key);
			continue;
		}
		// Cyclic publishes one chunk at a time from acquire(), so the enabled channels are
		// left to it — but a disabled one still has to lose its key, which is why this runs
		// at all in that mode.
		if(slot.cyclic) {
			continue;
		}
		// copy(), so the buffer is the whole history rather than one chunk of a growing
		// stream, and no depth claim can truncate the file.
		acq::SampleBuffer buf;
		buf.setCapacity(1);
		buf.push(slot.reader->samples(i));
		writes.append({key, std::move(buf)});
	}
}

void FileSourceBlock::collectChunk(Slot &slot, Chunks &chunks, Keys &drops) const
{
	const int total = slot.sampleCount();
	if(total <= 0) {
		return;
	}
	if(slot.playhead >= total) {
		slot.playhead = 0;
	}
	const int n = qMin(int(bufferSize()), total - slot.playhead);

	for(int i = 0; i < slot.channels.size(); ++i) {
		const acq::DataKey key = acq::DataKey::raw(id(), slot.channels.at(i));
		if(!isChannelEnabled(slot.channels.at(i))) {
			drops.append(key);
			continue;
		}
		// One slice, copied out: the reader's vector stays whole and the store gets a chunk
		// the size a live source would have written.
		chunks.append({key, slot.reader->samples(i).mid(slot.playhead, n)});
	}
	slot.playhead = (slot.playhead + n) % total;
}

void FileSourceBlock::apply(scopy::acq::DataStore *store, const Keys &drops, Writes &writes, Chunks &chunks) const
{
	// Done with m_mutex released: DataStore emits keysChanged, whose GUI handler calls back
	// into streamInfo().
	for(const acq::DataKey &key : drops) {
		store->remove(key);
	}
	for(auto &entry : writes) {
		store->copy(entry.first, std::move(entry.second));
	}
	// write(), not copy(): the store applies the key's depth claims and consumers see a stream
	// arriving, exactly as from a live source.
	for(auto &entry : chunks) {
		store->write(entry.first, std::move(entry.second));
	}
}

void FileSourceBlock::publish()
{
	if(m_store) {
		publishTo(m_store);
	}
}

void FileSourceBlock::publishTo(scopy::acq::DataStore *store)
{
	Writes writes;
	Keys drops;
	{
		QMutexLocker lk(&m_mutex);
		for(const std::unique_ptr<Slot> &slot : m_slots) {
			if(slot->reader) {
				collectWhole(*slot, writes, drops);
			}
		}
	}

	Chunks none;
	apply(store, drops, writes, none);
	Q_EMIT published();
}

void FileSourceBlock::onStart()
{
	SourceBlock::onStart();
	QMutexLocker lk(&m_mutex);
	for(std::unique_ptr<Slot> &slot : m_slots) {
		slot->playhead = 0;
		slot->publishedThisRun = false;
	}
}

void FileSourceBlock::acquire(scopy::acq::DataStore *store)
{
	if(m_stopRequested) {
		return;
	}

	Writes writes;
	Chunks chunks;
	Keys drops;
	{
		QMutexLocker lk(&m_mutex);
		for(std::unique_ptr<Slot> &slot : m_slots) {
			if(!slot->reader) {
				continue;
			}
			if(slot->cyclic) {
				collectChunk(*slot, chunks, drops);
				continue;
			}
			// Once per run and never again: the whole file is static, and run() clears the
			// store, so the copy made on the GUI thread is gone by the first cycle.
			if(slot->publishedThisRun) {
				continue;
			}
			slot->publishedThisRun = true;
			collectWhole(*slot, writes, drops);
		}
	}

	apply(store, drops, writes, chunks);
}

std::optional<scopy::acq::StreamInfo> FileSourceBlock::streamInfo(const scopy::acq::DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	if(key.sourceId() != id() || !key.isRaw()) {
		return std::nullopt;
	}

	for(const std::unique_ptr<Slot> &slot : m_slots) {
		const int index = slot->channels.indexOf(key.channelId());
		if(index < 0) {
			continue;
		}
		scopy::acq::StreamInfo info;
		info.kind = scopy::acq::ReprKind::Curve;
		info.label = slot->channels.at(index);
		info.unit = slot->reader ? slot->reader->unit(index) : QString();
		info.sampleRate = slot->rate();
		info.colorIndex = index;
		return info;
	}
	return std::nullopt;
}

QWidget *FileSourceBlock::createSettingsWidget(QWidget *parent)
{
	// The rows are a view of a slot list the reader edits while the instrument is alive, so the
	// widget needs nothing but the block.
	return withBaseSettings(new FileSourceWidget(this), parent);
}

// No isAvailable() override: this source reads files, so it is there whether or not a context was
// opened, and it ignores the one it is handed.
static const bool s_fileSourceRegistered =
	AcqSourceRegistry::instance().add([](iio_context *, QObject *parent) -> scopy::acq::SourceBlock * {
		auto *src = new FileSourceBlock(QStringLiteral("file"), parent);
		// One empty slot, so the panel opens on something to configure rather than on a bare
		// "add" button.
		src->addSlot();
		return src;
	});

#include "moc_filesourceblock.cpp"
