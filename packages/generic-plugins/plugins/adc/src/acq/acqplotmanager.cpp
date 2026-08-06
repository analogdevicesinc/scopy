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

#include "acqchannel.h"
#include "acqcurverepr.h"
#include "acqdigitalrepr.h"
#include "acqplotrow.h"
#include "acqwaterfallrepr.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>

#include <gui/plotaxis.h>
#include <gui/plotwidget.h>

#include <QLoggingCategory>
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
				// Every numeric conversion returns empty for annotations, so a curve
				// or a digital track here would silently draw nothing. Refuse
				// visibly instead; an AnnotationRepr is the eventual answer.
				qWarning(CAT_ACQ_PLOTMANAGER)
					<< "no representation for annotation stream" << key.toString();
				return nullptr;
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

	// After attach(): the settings page binds to the repr's PlotChannel and to the
	// row's axis, neither of which exists before it.
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
