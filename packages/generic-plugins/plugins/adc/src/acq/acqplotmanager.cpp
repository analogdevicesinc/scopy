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

#include "acqplotmanager.h"

#include "acqannotationrepr.h"
#include "acqchannel.h"
#include "acqcurverepr.h"
#include "acqdigitalrepr.h"
#include "acqplotrow.h"
#include "acqwaterfallrepr.h"
#include "measurementcontroller.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>

#include <gui/cursorcontroller.h>
#include <gui/plotaxis.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/menusectionwidget.h>

#include <QListWidget>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

#include <numeric>

Q_LOGGING_CATEGORY(CAT_ACQ_PLOTMANAGER, "AcqPlotManager")

using namespace scopy;
using namespace scopy::adc;

namespace {

// ~60 Hz. A cycle can complete far faster than this; the dirty flag collapses the
// extra cycles into one repaint.
constexpr int kFrameIntervalMs = 16;

// Menu ids for the two pages the manager owns itself, as opposed to one per channel.
constexpr const char *kCursorPageId = "acq-cursors";
constexpr const char *kKeyPickerPageId = "acq-keys";

} // namespace

AcqPlotManager::AcqPlotManager(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
			       InstrumentTemplate *shell, QWidget *parent)
	: QWidget(parent)
	, m_store(store)
	, m_engine(engine)
	, m_shell(shell)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);

	m_splitter = new QSplitter(Qt::Vertical, this);
	lay->addWidget(m_splitter);

	// Row 0 hosts every repr that draws an item. Reprs that own a whole plot
	// (waterfall) get appended as their own rows.
	m_sharedRow = addRow(new PlotWidget(this), /*exclusive=*/false);

	rebuildIndexRamp();
	// setPlotSize() would early-return on the unchanged value, so the initial X range
	// has to be set by hand — otherwise the first frame draws against Qwt's default.
	if(PlotWidget *p = sharedPlot()) {
		p->xAxis()->setInterval(0, m_plotSize - 1);
	}

	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, [this]() {
		if(!m_dirty) {
			return;
		}
		m_dirty = false;
		replot();
	});

	if(!m_store.isNull()) {
		// Emitted from whichever thread wrote, i.e. the worker — queued is mandatory.
		connect(m_store.data(), &scopy::acq::DataStore::keysChanged, this, &AcqPlotManager::onKeysChanged,
			Qt::QueuedConnection);
	}
}

AcqPlotManager::~AcqPlotManager()
{
	// Channels are parented to this, so each destructor detaches its repr and
	// releases its own claimant. Detach them explicitly first anyway: child
	// destruction order is unspecified and a repr must not touch a plot that has
	// already gone.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->detach();
	}
}

PlotWidget *AcqPlotManager::sharedPlot() const { return m_sharedRow ? m_sharedRow->plot() : nullptr; }

AcqPlotRow *AcqPlotManager::addRow(PlotWidget *plot, bool exclusive)
{
	if(!plot) {
		return nullptr;
	}
	m_splitter->addWidget(plot);

	// Row 0 (the item plot) gets twice the height of anything stacked under it —
	// the proportions src/sim/siminstrument.cpp:85-95 uses for plot-over-waterfall.
	const int idx = m_splitter->indexOf(plot);
	m_splitter->setStretchFactor(idx, idx == 0 ? 2 : 1);

	AcqPlotRow *row = new AcqPlotRow(plot, exclusive, this);
	m_rows.append(row);
	return row;
}

std::unique_ptr<AcqChannelRepr> AcqPlotManager::makeRepr(const scopy::acq::DataKey &key, ReprKind kind)
{
	if(kind == ReprKind::Auto) {
		if(m_store.isNull()) {
			return nullptr;
		}
		const std::optional<scopy::acq::SampleType> t = m_store->typeOf(key);
		if(!t.has_value()) {
			// Nothing has ever been written to this key, so there is no type to read.
			// Default to a curve rather than refuse: it is the only representation
			// every numeric type can use.
			kind = ReprKind::Curve;
		} else {
			switch(*t) {
			case scopy::acq::SampleType::Annotation:
				// The one type Auto can resolve exactly rather than guess: an
				// annotation stream has no numeric conversion at all (toFloat and
				// toBits both return empty for it), so AnnotationRepr is not a
				// heuristic, it is the only representation that can read it.
				kind = ReprKind::Annotations;
				break;
			case scopy::acq::SampleType::UInt8:
			case scopy::acq::SampleType::Int8:
				// A heuristic, not a truth: an 8-bit ADC stream would land here too.
				// The explicit-kind argument is the escape hatch.
				kind = ReprKind::Digital;
				break;
			default:
				kind = ReprKind::Curve;
				break;
			}
		}
	}

	// Auto cannot land here — it resolves annotations to AnnotationRepr — but an
	// explicit kind can, and this is the one combination that fails silently rather
	// than visibly: toFloat and toBits both return empty for an annotation stream, so
	// a curve or a digital track pointed at one draws nothing and looks like a dead
	// key. Refuse with a warning instead.
	if(kind != ReprKind::Annotations && !m_store.isNull()) {
		const std::optional<scopy::acq::SampleType> t = m_store->typeOf(key);
		if(t.has_value() && *t == scopy::acq::SampleType::Annotation) {
			qWarning(CAT_ACQ_PLOTMANAGER)
				<< "key" << key.toString()
				<< "is an annotation stream; only ReprKind::Annotations can read it";
			return nullptr;
		}
	}

	switch(kind) {
	case ReprKind::Curve: {
		auto repr = std::make_unique<CurveRepr>();
		repr->setIndexSource(&m_indexX);
		return repr;
	}
	case ReprKind::Digital:
		// No index source: the item lays its samples out proportionally across the
		// shared X axis from the sample count pull() gives it, so it needs no ramp.
		return std::make_unique<DigitalRepr>();
	case ReprKind::Waterfall:
		// Never picked by Auto: any Float32 stream converts, but a spectrogram of a
		// time-domain stream is noise. It has to be asked for.
		return std::make_unique<WaterfallRepr>();
	case ReprKind::Annotations:
		return std::make_unique<AnnotationRepr>();
	case ReprKind::Auto:
		break;
	}
	return nullptr;
}

AcqChannel *AcqPlotManager::addChannel(const scopy::acq::DataKey &key, const QString &name, const QColor &color,
				       ReprKind kind)
{
	std::unique_ptr<AcqChannelRepr> repr = makeRepr(key, kind);
	if(!repr) {
		return nullptr;
	}

	// A repr that *is* a plot gets its own row; everything else joins row 0. This is
	// the whole of the waterfall asymmetry — the manager never names a PlotWidget
	// subclass.
	AcqPlotRow *row = m_sharedRow;
	if(PlotWidget *own = repr->createOwnPlot(this)) {
		row = addRow(own, /*exclusive=*/true);
	}
	if(!row) {
		return nullptr;
	}

	AcqChannel *ch = new AcqChannel(m_store.data(), key, name, color, std::move(repr), m_uidCounter++, this);
	m_channels.append(ch);
	ch->attach(row);

	// Claim now, even though the key may not exist yet: DataStore::write() applies
	// pending claims before the first push, so an early claim is what makes the very
	// first window full-depth instead of a single chunk.
	reclaim(ch);

	connect(ch, &AcqChannel::depthNeedsReclaim, this, [this, ch]() { reclaim(ch); });

	// Before the claim would be more natural for the X key — a curve with an X stream
	// reads two keys — but the claim here is for this channel's own key only, so the
	// order does not matter. Before registerRail so the settings page sees a
	// fully-configured repr.
	applyCurveDefaults(ch);

	// Both after attach(): the settings page binds to the repr's PlotChannel and the
	// row's axis, and the measure manager is created in attach() too (it needs the
	// pen colour).
	registerMeasurements(ch);
	registerRail(ch);

	return ch;
}

void AcqPlotManager::removeChannel(AcqChannel *ch)
{
	if(!ch || !m_channels.removeOne(ch)) {
		return;
	}
	unregisterRail(ch);
	// The destructor detaches and releases the claim.
	delete ch;
}

void AcqPlotManager::registerRail(AcqChannel *ch)
{
	if(!ch || m_shell.isNull() || m_railRows.contains(ch)) {
		return;
	}

	if(!m_railGroup) {
		m_railGroup = m_shell->addChannelGroup(QStringLiteral("Plots"));
	}

	const QString id = ch->menuId();
	MenuControlButton *row = m_shell->addChannelSwitchRow(m_railGroup, ch->name(), ch->color(), id);
	m_railRows.insert(ch, row);

	// Built once, here, and never rebuilt — which is only sound because the key and
	// the repr are fixed for the channel's life.
	m_shell->addMenuPage(id, ch->createSettingsPage(m_shell.data()));

	if(SmallOnOffSwitch *sw = InstrumentTemplate::rowSwitch(row)) {
		connect(sw, &QAbstractButton::toggled, ch, &AcqChannel::setEnabled);
		// Both directions: a channel can be disabled from code (a vanished key), and
		// the switch has to show it.
		connect(ch, &AcqChannel::enabledChanged, sw, [sw](bool en) {
			QSignalBlocker b(sw);
			sw->setChecked(en);
		});
		sw->setChecked(ch->isEnabled());
	}

	// The rail row is the channel's name in the UI, so a rename from the settings page
	// has to reach it.
	connect(ch, &AcqChannel::nameChanged, row, [row](const QString &n) { row->setName(n); });
	connect(ch, &AcqChannel::colorChanged, row, [row](const QColor &c) { row->setColor(c); });
}

void AcqPlotManager::unregisterRail(AcqChannel *ch)
{
	if(!ch) {
		return;
	}
	MenuControlButton *row = m_railRows.take(ch);
	if(!row || m_shell.isNull()) {
		return;
	}
	// Four steps, done by the shell: button group, layout, menu page, deleteLater.
	m_shell->removeChannelRow(m_railGroup, row, ch->menuId());
}

void AcqPlotManager::setSampleRate(double sr)
{
	if(sr <= 0.0) {
		return;
	}
	m_sampleRate = sr;
	// Also to the channels that already exist, so the controller may call this before
	// or after any of them is created without the result differing.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		applyCurveDefaults(ch);
	}
}

void AcqPlotManager::setXKeyFor(const scopy::acq::DataKey &key, const scopy::acq::DataKey &xKey)
{
	if(key.key.isEmpty() || xKey.key.isEmpty()) {
		return;
	}
	m_xKeys.insert(key, xKey);
	for(AcqChannel *ch : std::as_const(m_channels)) {
		applyCurveDefaults(ch);
	}
}

void AcqPlotManager::applyCurveDefaults(AcqChannel *ch)
{
	if(!ch || !ch->repr()) {
		return;
	}
	// Only a curve has either. A dynamic_cast rather than reading kindName(): the two
	// setters are CurveRepr's own, not part of the repr interface, so this is the one
	// place in the manager that has to know a concrete repr type — and a failed cast
	// is exactly the right answer for a digital track or a waterfall.
	CurveRepr *curve = dynamic_cast<CurveRepr *>(ch->repr());
	if(!curve) {
		return;
	}
	curve->setSampleRate(m_sampleRate);
	// Only when one is registered: setXKey with an empty key would be a no-op anyway,
	// but leaving an already-set key alone matters if a reader ever sets one by hand.
	const auto it = m_xKeys.constFind(ch->key());
	if(it != m_xKeys.constEnd()) {
		curve->setXKey(it.value());
	}
}

MeasurementsPanel *AcqPlotManager::measurePanel()
{
	if(m_measurePanel.isNull() && !m_shell.isNull()) {
		m_measurePanel = new MeasurementsPanel(this);
		// Both hide themselves when their label list empties
		// (gui/src/widgets/measurementpanel.cpp:174-176), so starting hidden keeps the
		// two states consistent — otherwise an untouched instrument reserves plot
		// height for an empty strip that would never come back once hidden.
		m_measurePanel->setVisible(false);
		m_shell->addToSlot(PS_BOTTOM, m_measurePanel);
	}
	return m_measurePanel.data();
}

StatsPanel *AcqPlotManager::statsPanel()
{
	if(m_statsPanel.isNull() && !m_shell.isNull()) {
		m_statsPanel = new StatsPanel(this);
		m_statsPanel->setVisible(false);
		// StatsPanel asks for Expanding in both directions, which is right for the side
		// slot it was written for and wrong above a plot: PS_TOP is an unstretched
		// layout, so the panel would still claim height off its scroll area's size hint
		// and push the plot down. Maximum vertically, the same policy MeasurementsPanel
		// picks for itself (gui/src/widgets/measurementpanel.cpp:44).
		m_statsPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		// PS_TOP, above the plot, with the measurement readouts below it. Stats are the
		// min/max/avg of the measurements — reading the two against each other means
		// both laid out the same way, not one of them turned sideways in the right slot.
		m_shell->addToSlot(PS_TOP, m_statsPanel);
	}
	return m_statsPanel.data();
}

void AcqPlotManager::registerMeasurements(AcqChannel *ch)
{
	if(!ch || !ch->repr()) {
		return;
	}
	MeasureManagerInterface *mgr = ch->repr()->measureManager();
	if(!mgr) {
		// A digital track or a waterfall. Nothing to measure, and no panel to create
		// for it — which is why the panels are built lazily here rather than in the
		// constructor.
		return;
	}

	MeasurementsPanel *meas = measurePanel();
	StatsPanel *stats = statsPanel();
	if(!meas || !stats) {
		return;
	}

	connect(mgr, &MeasureManagerInterface::enableMeasurement, meas, &MeasurementsPanel::addMeasurement);
	connect(mgr, &MeasureManagerInterface::disableMeasurement, meas, &MeasurementsPanel::removeMeasurement);
	// The panel is shown by the first label rather than up front, and never hidden
	// here: it hides itself when the last label goes.
	connect(mgr, &MeasureManagerInterface::enableMeasurement, meas, [meas]() { meas->setVisible(true); });

	connect(mgr, &MeasureManagerInterface::enableStat, stats, &StatsPanel::addStat);
	connect(mgr, &MeasureManagerInterface::disableStat, stats, &StatsPanel::removeStat);
	connect(mgr, &MeasureManagerInterface::enableStat, stats, [stats]() { stats->setVisible(true); });

	// The panels' own "hide all" button unchecks every selector box, which is what
	// actually removes the labels — the button alone would leave the model measuring.
	// inhibitUpdates around it because removeMeasurement relayouts the whole stack per
	// label otherwise (measurementpanel.cpp:178-182).
	connect(meas, &MeasurementsPanel::hideAll, mgr, [meas, mgr]() {
		meas->setInhibitUpdates(true);
		Q_EMIT mgr->toggleAllMeasurement(false);
		meas->setInhibitUpdates(false);
	});
	connect(stats, &StatsPanel::hideAll, mgr, [mgr]() { Q_EMIT mgr->toggleAllStats(false); });
}

CursorController *AcqPlotManager::cursors(QString *settingsId)
{
	if(settingsId) {
		*settingsId = QString::fromLatin1(kCursorPageId);
	}
	if(!m_cursors.isNull()) {
		return m_cursors.data();
	}
	PlotWidget *plot = sharedPlot();
	if(!plot || m_shell.isNull()) {
		return nullptr;
	}

	m_cursors = new CursorController(plot, this);
	CursorSettings *settings = new CursorSettings(this);
	m_cursors->connectSignals(settings);
	m_shell->addMenuPage(QString::fromLatin1(kCursorPageId), settings);
	// Cursors off until asked for: the handles are created either way, but a plot that
	// opens with four cursors on it is not what anyone wants.
	m_cursors->setVisible(false);
	return m_cursors.data();
}

QString AcqPlotManager::createKeyPickerPage()
{
	const QString id = QString::fromLatin1(kKeyPickerPageId);
	if(m_shell.isNull() || m_shell->hasMenuPage(id)) {
		return id;
	}

	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	// SO_VIEW: adding a plot channel reads a key, it does not configure a block.
	MenuSectionCollapseWidget *section = m_shell->createMenuSection(QStringLiteral("ADD CHANNEL"), SO_VIEW, page);

	m_keyList = new QListWidget(section);
	m_keyList->setSelectionMode(QAbstractItemView::SingleSelection);
	section->add(m_keyList);

	m_keyKindCombo = new MenuCombo(QStringLiteral("Representation"), section);
	// Auto first, and it is finally honest here: every key in this list has been
	// written at least once, so DataStore::typeOf() can answer and makeRepr's
	// type switch is a real decision rather than a fallback to Curve.
	m_keyKindCombo->combo()->addItem(QStringLiteral("Auto"), static_cast<int>(ReprKind::Auto));
	m_keyKindCombo->combo()->addItem(QStringLiteral("Curve"), static_cast<int>(ReprKind::Curve));
	m_keyKindCombo->combo()->addItem(QStringLiteral("Digital"), static_cast<int>(ReprKind::Digital));
	m_keyKindCombo->combo()->addItem(QStringLiteral("Waterfall"), static_cast<int>(ReprKind::Waterfall));
	m_keyKindCombo->combo()->addItem(QStringLiteral("Annotations"), static_cast<int>(ReprKind::Annotations));
	section->add(m_keyKindCombo);

	QPushButton *addBtn = new QPushButton(QObject::tr("Add"), section);
	Style::setStyle(addBtn, style::properties::button::basicButton);
	section->add(addBtn);
	connect(addBtn, &QPushButton::clicked, this, [this]() {
		if(m_keyList.isNull() || !m_keyList->currentItem() || m_keyKindCombo.isNull()) {
			return;
		}
		const scopy::acq::DataKey key(m_keyList->currentItem()->text());
		const ReprKind kind = static_cast<ReprKind>(m_keyKindCombo->combo()->currentData().toInt());
		// Name guessed from the key's channel segment, colour cycled by channel count.
		// Both are editable from the new channel's own page, so guessing is fine — and
		// channelId() is empty for a key that does not follow the source_channel_stage
		// convention, hence the fallback to the whole key.
		const QString chId = key.channelId();
		AcqChannel *ch = addChannel(key, chId.isEmpty() ? key.toString() : chId,
					    Style::getChannelColor(m_channels.count()), kind);
		if(!ch) {
			// makeRepr refused. Only reachable by naming a kind that cannot read the
			// key — a Curve or Digital on an annotation stream — since Auto resolves
			// every type there is.
			return;
		}
		// Straight to the new channel's page: the guessed name and colour are the first
		// things a reader will want to change.
		if(!m_shell.isNull()) {
			m_shell->showMenuPage(ch->menuId());
		}
	});

	lay->addWidget(section);
	lay->addStretch();

	m_shell->addMenuPage(id, page);
	// Pinned above the groups rather than inside one: it is not a channel, and it has
	// to be reachable before any channel exists.
	m_shell->addRailHeaderRow(QStringLiteral("Add channel"), id);

	refreshKeyPicker();
	return id;
}

void AcqPlotManager::refreshKeyPicker()
{
	if(m_keyList.isNull() || m_store.isNull()) {
		return;
	}
	// Rebuilt wholesale rather than diffed: the list is short, and the selection is
	// only meaningful at the moment Add is pressed. Preserve it anyway so a key
	// arriving mid-choice does not move the target out from under the pointer.
	const QString selected = m_keyList->currentItem() ? m_keyList->currentItem()->text() : QString();

	m_keyList->clear();
	const QList<scopy::acq::DataKey> keys = m_store->keys();
	for(const scopy::acq::DataKey &k : keys) {
		m_keyList->addItem(k.toString());
	}
	if(!selected.isEmpty()) {
		const QList<QListWidgetItem *> found = m_keyList->findItems(selected, Qt::MatchExactly);
		if(!found.isEmpty()) {
			m_keyList->setCurrentItem(found.first());
		}
	}
}

void AcqPlotManager::rebuildIndexRamp()
{
	m_indexX.resize(m_plotSize);
	std::iota(m_indexX.begin(), m_indexX.end(), 0.0f);
}

void AcqPlotManager::setPlotSize(int n)
{
	n = qMax(1, n);
	if(n == m_plotSize) {
		return;
	}
	m_plotSize = n;
	rebuildIndexRamp();

	if(PlotWidget *p = sharedPlot()) {
		p->xAxis()->setInterval(0, m_plotSize - 1);
	}
	reclaimAll();
}

void AcqPlotManager::reclaim(AcqChannel *ch)
{
	if(!ch) {
		return;
	}
	// Safe to read from the GUI thread: m_bufferSize is deliberately non-atomic
	// because the UI locks the buffer control while running, so it only ever changes
	// while the worker is stopped.
	const std::size_t bufSize = m_engine.isNull() ? 1u : m_engine->bufferSize();
	ch->reclaimDepth(m_plotSize, bufSize);
}

void AcqPlotManager::reclaimAll()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		reclaim(ch);
	}
}

void AcqPlotManager::onBufferSizeChanged() { reclaimAll(); }

void AcqPlotManager::onKeysChanged(QList<scopy::acq::DataKey> keys)
{
	// Unconditionally, not a diff: DataStore::reset() and remove() erase m_claims
	// wholesale, so after either every channel's claim is gone and must be
	// re-registered.
	reclaimAll();
	Q_EMIT keysAvailable(keys);
}

void AcqPlotManager::onCycleComplete()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->pull(m_plotSize);
	}
	// Deliberately no replot here — the frame timer owns repainting.
	m_dirty = true;
}

void AcqPlotManager::onStarted()
{
	// run()/single() clear the store's chunks but keep the claims, and clearing
	// chunks does not blank a curve: it would keep drawing the previous run's tail
	// until the first new cycle landed.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->reset();
	}
	m_dirty = true;
	m_frameTimer->start();
}

void AcqPlotManager::onStopped()
{
	m_frameTimer->stop();
	// One final flush, so the last cycle before the stop is not left unpainted.
	if(m_dirty) {
		m_dirty = false;
		replot();
	}
}

void AcqPlotManager::replot()
{
	for(AcqPlotRow *r : std::as_const(m_rows)) {
		r->replot();
	}
	Q_EMIT newData();
}

#include "moc_acqplotmanager.cpp"
