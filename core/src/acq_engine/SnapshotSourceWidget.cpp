#include "SnapshotSourceWidget.h"

#include "AcquisitionEngine.h"
#include "DataKeyCombo.h"
#include "DataStore.h"

#include <gui/style.h>
#include <gui/widgets/menusectionwidget.h>

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::acq;

namespace {

// Matches GenalyzerTransformSettings' layout so the block panels line up.
QWidget *labelledRow(const QString &text, QWidget *field, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);

	auto *label = new QLabel(text, w);
	Style::setStyle(label, style::properties::label::subtle);

	lay->addWidget(label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(field);
	return w;
}

// Selects `key`, inserting it as a visibly stale entry when the key set no longer offers it.
// populateKeyCombo() preserves whatever the box had, but a selection whose stream has gone would
// fall back to index 0 and silently retarget the slot.
void selectKey(QComboBox *box, const DataKey &key)
{
	if(!box || key.key.isEmpty()) {
		return;
	}
	QSignalBlocker b(box);
	if(box->findData(key.key) < 0) {
		box->insertItem(
			0, QCoreApplication::translate("SnapshotSourceWidget", "%1 (absent)").arg(key.key),
			key.key);
	}
	box->setCurrentIndex(box->findData(key.key));
}

// Frame 1 of N rather than 0: the playhead is an implementation index.
QString playbackStatus(const SnapshotSource::Slot &s)
{
	return QCoreApplication::translate("SnapshotSourceWidget", "frozen: %1 — frame %2/%3 (%4 pts)")
		.arg(s.capturedFrom)
		.arg(s.playhead + 1)
		.arg(s.chunkCount())
		.arg(s.sampleCount());
}

// 10 Hz: fast enough to read as motion, slow enough that a cycle-rate recording does not
// redraw the label thousands of times a second. The counter may skip frames, which is correct
// for a readout — it is not a frame-accurate trace.
constexpr int kPollIntervalMs = 100;

} // namespace

SnapshotSourceWidget::SnapshotSourceWidget(SnapshotSource *src, DataStore *store, AcquisitionEngine *engine,
					   QWidget *parent)
	: QWidget(parent)
	, m_src(src)
	, m_store(store)
	, m_engine(engine)
{
	setupUI();

	// What QPointer used to do. This widget is owned by the menu page's layout while the block
	// is parented to the engine, so a teardown destroys the block first — and m_poll would
	// otherwise tick into it.
	if(m_src) {
		connect(m_src, &SnapshotSource::slotsChanged, this, &SnapshotSourceWidget::rebuildRows);
		connect(m_src, &SnapshotSource::slotCaptured, this, &SnapshotSourceWidget::syncRowStatus);
		connect(m_src, &QObject::destroyed, this, [this]() {
			m_src = nullptr;
			m_poll->stop();
		});
	}
	if(m_store) {
		connect(m_store, &QObject::destroyed, this, [this]() { m_store = nullptr; });
	}
	if(m_engine) {
		connect(m_engine, &QObject::destroyed, this, [this]() { m_engine = nullptr; });
	}

	rebuildRows();
}

void SnapshotSourceWidget::setupUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	auto *section = new MenuSectionWidget(this);
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(8);

	m_emptyHint = new QLabel(tr("No snapshots. Add one, pick a stream, then capture."), section);
	m_emptyHint->setWordWrap(true);
	Style::setStyle(m_emptyHint, style::properties::label::subtle);
	section->contentLayout()->addWidget(m_emptyHint);

	m_rowsLay = new QVBoxLayout();
	m_rowsLay->setContentsMargins(0, 0, 0, 0);
	m_rowsLay->setSpacing(6);
	section->contentLayout()->addLayout(m_rowsLay);

	auto *addBtn = new QPushButton(tr("+ Add snapshot"), section);
	Style::setStyle(addBtn, style::properties::button::basicButton);
	connect(addBtn, &QPushButton::clicked, this, [this]() {
		if(m_src) {
			m_src->addSlot();
		}
	});
	section->contentLayout()->addWidget(addBtn);

	mainLayout->addWidget(section);

	// syncRowStatus() already reads the whole row state from the block, so the poll needs no
	// formatting code of its own.
	m_poll = new QTimer(this);
	m_poll->setInterval(kPollIntervalMs);
	connect(m_poll, &QTimer::timeout, this, [this]() {
		// A hidden menu page has nothing to update, and the block keeps running.
		if(!isVisible() || !m_src) {
			return;
		}
		for(int i = 0; i < m_rows.size(); ++i) {
			syncRowStatus(i);
		}
	});
	m_poll->start();
}

void SnapshotSourceWidget::rebuildRows()
{
	// Guarded for the whole rebuild: populating the widgets fires the same signals a reader's
	// edit does, and a half-built row must not write itself back.
	m_building = true;

	QLayoutItem *item;
	while((item = m_rowsLay->takeAt(0)) != nullptr) {
		if(QWidget *w = item->widget()) {
			w->deleteLater();
		}
		delete item;
	}
	m_rows.clear();

	// Not `slots` — that is a Qt macro.
	const QList<SnapshotSource::Slot> list = m_src ? m_src->allSlots() : QList<SnapshotSource::Slot>();
	for(int i = 0; i < list.size(); ++i) {
		m_rowsLay->addWidget(buildRow(i, list.at(i)));
	}
	m_emptyHint->setVisible(list.isEmpty());

	m_building = false;

	// After the guard clears, so the labels read the block rather than the widgets that were
	// just being populated.
	for(int i = 0; i < m_rows.size(); ++i) {
		syncRowStatus(i);
		syncKeyHint(i);
	}
}

QWidget *SnapshotSourceWidget::buildRow(int index, const SnapshotSource::Slot &s)
{
	auto *frame = new QFrame(this);
	frame->setFrameShape(QFrame::StyledPanel);
	auto *lay = new QVBoxLayout(frame);
	lay->setContentsMargins(6, 6, 6, 6);
	lay->setSpacing(4);

	Row row;

	// The name comes first: it is mandatory, and the only control usable before a source is
	// picked. Placeholder rather than a default, or every snapshot would share a name.
	auto *header = new QWidget(frame);
	auto *headerLay = new QHBoxLayout(header);
	headerLay->setContentsMargins(0, 0, 0, 0);
	row.title = new QLineEdit(s.title, header);
	row.title->setPlaceholderText(QStringLiteral("snapshot_"));
	row.title->setClearButtonEnabled(true);
	headerLay->addWidget(row.title);
	auto *removeBtn = new QPushButton(tr("Remove"), header);
	Style::setStyle(removeBtn, style::properties::button::borderButton);
	connect(removeBtn, &QPushButton::clicked, this, [this, index]() {
		if(m_src) {
			m_src->removeSlot(index);
		}
	});
	headerLay->addWidget(removeBtn);
	lay->addWidget(header);

	row.keyHint = new QLabel(frame);
	row.keyHint->setWordWrap(true);
	Style::setStyle(row.keyHint, style::properties::label::subtle);
	lay->addWidget(row.keyHint);

	// No sample-index entry: a slot freezes a real stream, and a frozen abscissa is a second
	// slot.
	row.source = new QComboBox(frame);
	populateKeyCombo(row.source, m_store, m_engine, /*withSampleIndex=*/false);
	selectKey(row.source, s.source);
	lay->addWidget(labelledRow(tr("Source:"), row.source, frame));

	row.cyclicBox = new QCheckBox(tr("Cyclic mode"), frame);
	row.cyclicBox->setChecked(s.cyclic);
	row.cyclicBox->setToolTip(tr("Hand out one captured chunk per acquisition cycle, looping at "
				     "the end. Off publishes the whole captured history at once."));
	lay->addWidget(row.cyclicBox);

	row.status = new QLabel(frame);
	row.status->setWordWrap(true);
	Style::setStyle(row.status, style::properties::label::subtle);
	lay->addWidget(row.status);

	auto *actions = new QWidget(frame);
	auto *actionsLay = new QHBoxLayout(actions);
	actionsLay->setContentsMargins(0, 0, 0, 0);
	actionsLay->addStretch();
	row.captureBtn = new QPushButton(tr("Capture"), actions);
	Style::setStyle(row.captureBtn, style::properties::button::borderButton);
	connect(row.captureBtn, &QPushButton::clicked, this, [this, index]() {
		if(!m_src) {
			return;
		}
		// The reader may have typed a name or picked a source without leaving the widget,
		// and uncommitted state means no keys.
		commitTitle(index);
		pushRow(index);
		m_src->capture(index);
	});
	actionsLay->addWidget(row.captureBtn);
	row.clearBtn = new QPushButton(tr("Clear"), actions);
	Style::setStyle(row.clearBtn, style::properties::button::borderButton);
	connect(row.clearBtn, &QPushButton::clicked, this, [this, index]() {
		if(m_src) {
			m_src->clearSlot(index);
		}
	});
	actionsLay->addWidget(row.clearBtn);
	lay->addWidget(actions);

	connect(row.source, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, index](int) {
		if(!m_building) {
			pushRow(index);
		}
	});
	connect(row.cyclicBox, &QCheckBox::toggled, this, [this, index](bool on) {
		if(!m_building && m_src) {
			m_src->setSlotCyclic(index, on);
		}
	});
	// On commit, not per keystroke: setSlotTitle() renames the key and drops the old one, so
	// applying it per character would churn the store through every prefix typed.
	connect(row.title, &QLineEdit::editingFinished, this, [this, index]() {
		if(!m_building) {
			commitTitle(index);
		}
	});
	// The hint does follow every keystroke — it is the preview of what commit will do.
	connect(row.title, &QLineEdit::textChanged, this, [this, index](const QString &) {
		if(!m_building) {
			syncKeyHint(index);
		}
	});

	m_rows.append(row);
	return frame;
}

void SnapshotSourceWidget::commitTitle(int index)
{
	if(!m_src || index < 0 || index >= m_rows.size()) {
		return;
	}
	const Row &row = m_rows.at(index);
	const std::optional<SnapshotSource::Slot> before = m_src->slotAt(index);

	if(!m_src->setSlotTitle(index, row.title->text())) {
		// Rejected: empty, or a name another slot holds. Back to what the block actually has,
		// rather than showing a name the store does not know.
		QSignalBlocker b(row.title);
		row.title->setText(before ? before->title : QString());
	}
	syncKeyHint(index);
	syncRowStatus(index);
}

void SnapshotSourceWidget::syncKeyHint(int index)
{
	if(!m_src || index < 0 || index >= m_rows.size()) {
		return;
	}
	const Row &row = m_rows.at(index);
	const QString clean = SnapshotSource::sanitizeTitle(row.title->text());

	if(clean.isEmpty()) {
		row.keyHint->setText(tr("A name is required — it is what the stream is called."));
	} else if(!m_src->isTitleAvailable(clean, index)) {
		row.keyHint->setText(tr("\"%1\" is already taken by another snapshot.").arg(clean));
	} else {
		row.keyHint->setText(SnapshotSource::keyFor(m_src->name(), clean).toString());
	}
}

void SnapshotSourceWidget::pushRow(int index)
{
	if(m_src && index >= 0 && index < m_rows.size()) {
		m_src->setSlotSource(index, keyFromCombo(m_rows.at(index).source));
	}
}

void SnapshotSourceWidget::syncRowStatus(int index)
{
	if(!m_src || index < 0 || index >= m_rows.size()) {
		return;
	}
	const Row &row = m_rows.at(index);
	const std::optional<SnapshotSource::Slot> s = m_src->slotAt(index);
	if(!s) {
		return;
	}

	// Saying so on the button beats a press that only logs a warning.
	row.captureBtn->setEnabled(!s->title.isEmpty());

	if(!s->captured) {
		row.status->setText(s->title.isEmpty() ? tr("empty — name it first") : tr("empty"));
		row.clearBtn->setEnabled(false);
		// Left checked: it is a setting for the next capture, not a report.
		row.cyclicBox->setEnabled(false);
		return;
	}
	row.clearBtn->setEnabled(true);

	const int chunks = s->chunkCount();
	row.cyclicBox->setEnabled(chunks > 1); // a control over nothing for a single chunk

	if(s->cyclic && chunks > 1) {
		row.status->setText(playbackStatus(*s));
	} else if(chunks > 1) {
		row.status->setText(tr("frozen: %1 (%2 pts × %3 chunks)")
					    .arg(s->capturedFrom)
					    .arg(s->sampleCount())
					    .arg(chunks));
	} else {
		row.status->setText(tr("frozen: %1 (%2 pts)").arg(s->capturedFrom).arg(s->sampleCount()));
	}
}

void SnapshotSourceWidget::refreshKeys()
{
	// In place rather than a rebuild: populateKeyCombo preserves each selection, so a key
	// appearing mid-edit does not disturb the reader.
	m_building = true;
	for(int i = 0; i < m_rows.size(); ++i) {
		populateKeyCombo(m_rows.at(i).source, m_store, m_engine, /*withSampleIndex=*/false);
		// Re-asserted from the block: a selection whose stream has gone is the one case
		// populateKeyCombo cannot preserve.
		if(const std::optional<SnapshotSource::Slot> s = m_src ? m_src->slotAt(i) : std::nullopt) {
			selectKey(m_rows.at(i).source, s->source);
		}
	}
	m_building = false;
}

#include "moc_SnapshotSourceWidget.cpp"
