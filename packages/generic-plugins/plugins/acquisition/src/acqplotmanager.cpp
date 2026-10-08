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

#include "acqaxis.h"
#include "acqchannelregistry.h"
#include "acqplot.h"
#include "acqplotrail.h"
#include "acqtriggermarkerview.h"

#include <core/acq_engine/datastore.h>

// The factories, needed here and not in the header: they are static free functions, so
// the translation unit that calls one must include it (see the ODR note in
// gui/docking/docksettings.h).
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <QLoggingCategory>
#include <QSet>
#include <QTimer>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(CAT_ACQ_PLOTMANAGER, "AcqPlotManager")

using namespace scopy;
using namespace scopy::adc;

AcqPlotManager::AcqPlotManager(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
			       InstrumentTemplate *shell, QWidget *parent)
	: QWidget(parent)
	, m_kFrameIntervalMs(16)
	, m_store(store)
	, m_engine(engine)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);

	// A dock area rather than a splitter, so each plot arrives with a title bar and can
	// be dragged, tabbed, split and resized — the same container TimePlotComponent and
	// FFTPlotComponent use (src/time/timeplotcomponent.cpp:52-55).
	m_dockArea = createDockableArea(this);
	QWidget *areaWidget = m_dockArea->asWidget();
	Style::setBackgroundColor(areaWidget, json::theme::background_subtle, true);
	lay->addWidget(areaWidget);

	// No plot is created here, and that is the point: an empty manager is a valid state
	// and the reader adds the first plot from the rail. A plot created in this
	// constructor would be the manager deciding what the instrument shows.
	// The rail is built now, though: it holds the "Add plot" row, which is the only
	// way to make the first plot.
	m_rail = new AcqPlotRail(shell, store, engine, this);
	connect(m_rail, &AcqPlotRail::addPlotRequested, this, [this](const QString &name, AcqPlotKind kind) {
		addPlot(name.isEmpty() ? tr("Plot %1").arg(m_plotUuidCounter) : name, kind);
	});
	connect(m_rail, &AcqPlotRail::addChannelRequested, this,
		[this](AcqPlot *p, scopy::acq::ReprKind kind, const scopy::acq::DataKey &yKey,
		       const scopy::acq::DataKey &xKey) { addChannel(p, kind, yKey, xKey); });
	connect(m_rail, &AcqPlotRail::removePlotRequested, this, &AcqPlotManager::removePlot);

	m_trigMarkerView = new AcqTriggerMarkerView(this);
	connect(m_trigMarkerView, &AcqTriggerMarkerView::windowChanged, this, &AcqPlotManager::triggerWindowChanged);

	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(m_kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, &AcqPlotManager::flushIfDirty);

	if(!m_store.isNull()) {
		// Emitted from whichever thread wrote, i.e. the worker — queued is mandatory.
		connect(m_store.data(), &scopy::acq::DataStore::keysChanged, this, &AcqPlotManager::onKeysChanged,
			Qt::QueuedConnection);
	}
}

AcqPlotManager::~AcqPlotManager()
{
	// Channels are parented to this, so each destructor detaches itself and releases its
	// own claimant. Detach them explicitly first anyway: child destruction order is
	// unspecified and a channel must not touch a plot that has already gone.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->detach();
	}
	// The AcqPlots are parented to this too, so Qt frees them; their widgets are children
	// of their docks, which are children of the dock area, which is a child of this.
}

// --- plots -----------------------------------------------------------------

AcqPlot *AcqPlotManager::plot(quint32 uuid) const
{
	for(AcqPlot *p : std::as_const(m_plots)) {
		if(p->uuid() == uuid) {
			return p;
		}
	}
	return nullptr;
}

PlotWidget *AcqPlotManager::sharedPlot() const { return m_plots.isEmpty() ? nullptr : m_plots.first()->plot(); }

AcqPlot *AcqPlotManager::addPlot(const QString &name, AcqPlotKind kind)
{
	const quint32 uuid = m_plotUuidCounter++;
	AcqPlot *p = new AcqPlot(name, kind, uuid, this);

	// Direction_BOTTOM, not the interface's Direction_RIGHT default: plots stack
	// vertically, and in the plain-layout backend the *first* direction is what picks
	// the orientation for every dock after it
	// (gui/src/docking/dockableareaclassic.cpp:66-74) — a right would give a row.
	//
	// No sizing is asked for. The dock area splits evenly and the reader drags the
	// separators; the splitter's old fixed 2:1 could not be recomputed on removal
	// anyway, so after deleting the first plot it lied.
	DockWrapperInterface *dock = createDockWrapper(p->name());
	// Again, explicitly: the KDDW wrapper's constructor sets the title through its own
	// space-prepending override, so the name it was built with arrives indented twice.
	dock->setTitle(p->name());
	dock->setInnerWidget(p->plot());
	m_dockArea->addDockWrapper(dock, DockableAreaInterface::Direction_BOTTOM);

	// The instrument's current window width, so a plot added later starts where the
	// spinbox already says rather than at AcqPlot's own default.
	p->setPlotSize(m_plotSize);

	m_plots.append(p);
	m_rail->registerPlot(p);
	m_docks.insert(p, dock);

	// The title bar is the only place the plot's name shows outside the rail, so it
	// follows a rename the way the FFT waterfall's does
	// (src/freq/fftplotcomponent.cpp:128-130).
	connect(p, &AcqPlot::nameChanged, this, [dock](const QString &n) { dock->setTitle(n); });

	// Everything derived from a plot's width, on the plot's own say-so rather than only on
	// the path that set it.
	connect(p, &AcqPlot::plotSizeChanged, this, [this]() {
		announceMaxWindowSize();
		applyWindowToIndexAxes();
		updateTriggerMarkerWindow();
		markDirty();
	});

	announceMaxWindowSize();
	Q_EMIT plotAdded(uuid);
	// After the emit: a handler may add channels to the new plot, and the resolve should see
	// them. Idempotent, so the channels' own retargets costing a second pass is harmless.
	retargetTriggerHandle();
	return p;
}

void AcqPlotManager::removePlot(quint32 uuid)
{
	AcqPlot *p = plot(uuid);
	if(!p) {
		return;
	}

	// First, while everything below it is still readable.
	Q_EMIT plotRemoved(uuid);

	// Over a copy: removeChannel mutates the plot's list through removeChannelRef.
	const QList<AcqChannel *> chans = p->channels();
	for(AcqChannel *ch : chans) {
		removeChannel(ch);
	}

	DockWrapperInterface *dock = m_docks.take(p);

	// After the channels, so their rows are unregistered from a container that still
	// exists.
	m_rail->unregisterPlot(p);
	m_plots.removeOne(p);

	// Nothing to do about the cursors here: they belong to the plot now, as children of
	// the AcqPlot, and go when it does. The settings page goes with the plot's menu page,
	// which the rail already took down.

	// The dock and not the plot widget: setInnerWidget reparented the widget into the
	// dock, so the dock is what has to go and the widget goes with it as its child.
	// Detaching the widget first would leave the dock holding a guest it no longer
	// contains, which is the backend's invariant and not ours to break.
	//
	// deleteLater rather than delete, for the reason the plot widget alone used to be
	// deferred: a pending paint event on the canvas has to finish first. And through the
	// QWidget side, because DockWrapperInterface has no virtual destructor — the same path
	// extprocplugin's PlotManager::clearPlots takes
	// (packages/extproc/plugins/extprocplugin/src/plotmanager/plotmanager.cpp:104-112).
	if(QWidget *dw = dynamic_cast<QWidget *>(dock)) {
		dw->deleteLater();
	} else if(PlotWidget *w = p->plot()) {
		// No dock to delete (a plot added before one existed): fall back to the widget.
		w->deleteLater();
	}
	// deleteLater and not delete, for the same reason: the pool's axes are children of this
	// object, and destroying it ahead of the widget would free axes the widget still lists.
	p->deleteLater();

	announceMaxWindowSize();
	// Last, with the plot already off m_plots: the resolve cannot find its axis and put the
	// marker straight back onto the plot that just went.
	retargetTriggerHandle();
	markDirty();
}

// --- channels --------------------------------------------------------------

AcqChannel *AcqPlotManager::addChannel(AcqPlot *p, scopy::acq::ReprKind kind, const scopy::acq::DataKey &yKey,
				       const scopy::acq::DataKey &xKey,
				       const std::optional<scopy::acq::StreamInfo> &info)
{
	if(m_store.isNull()) {
		return nullptr;
	}
	if(!p || !m_plots.contains(p)) {
		qWarning(CAT_ACQ_PLOTMANAGER) << "refusing" << yKey.toString() << ": no such plot";
		return nullptr;
	}
	// No compatibility test past this point — not the representation against the plot
	// kind, not a channel count, not a duplicate. See addChannel's declaration for why.

	AcqChannel::Args args;
	args.store = m_store.data();
	args.engine = m_engine.data();
	args.key = yKey;
	args.xKey = xKey;
	// The caller's override first, else the producing block's own declaration, else a
	// default-built descriptor — an unlabelled, unitless curve, which is all that can
	// honestly be said about a key no block describes.
	args.info = info.value_or(m_engine.isNull() ? scopy::acq::StreamInfo{}
						    : m_engine->streamInfo(yKey).value_or(scopy::acq::StreamInfo{}));
	args.uid = m_uidCounter++;
	args.parent = this;

	// The producer's slot if it named one, otherwise the next from our palette. Done here
	// rather than in the channel because it is the *view's* palette — a channel has no way
	// to know which slots its siblings took.
	if(args.info.colorIndex < 0) {
		args.info.colorIndex = m_nextColorIndex++;
	}
	// Record what the reader picked: a descriptor still saying Curve for a channel drawn
	// as a logic track would make the two indistinguishable in the rail and the log.
	args.info.kind = kind;

	AcqChannel *ch = AcqChannelRegistry::instance().create(kind, args);
	if(!ch) {
		// An unregistered kind. The registry has already warned naming it.
		return nullptr;
	}

	// Before attach(): a kind can bake the rate into whatever it builds there, and a
	// channel whose producer declared a rate ignores it anyway.
	ch->setFallbackSampleRate(m_fallbackSampleRate);

	m_channels.append(ch);
	p->addChannelRef(ch);
	ch->attach(p);

	// No setInterval for a sample-index axis any more: the ramp is a stream, so the
	// channel's own requestInterval(x[0], x[n-1]) sets that range on the same path as
	// every other X source — including when the reader retargets *back* to sample index,
	// which this hand-written call could never cover.

	// Claim now, even though the key may not exist yet: DataStore::write() applies pending
	// claims before the first push, so an early claim is what makes the very first window
	// full-depth instead of a single chunk.
	reclaim(ch);

	// A retarget leaves a stale claim on the key that dropped out. Releasing the whole
	// claimant and re-claiming is exactly equivalent to releasing `dropped` alone — a
	// channel only ever claims its own two keys — and stays correct if a kind ever starts
	// reading a third.
	connect(ch, &AcqChannel::depthNeedsReclaim, this, [this, ch](scopy::acq::DataKey dropped) {
		Q_UNUSED(dropped)
		if(!m_store.isNull()) {
			m_store->releaseClaimant(ch->claimant());
		}
		reclaim(ch);
		markDirty();
	});

	// Queued: the handler destroys this channel, and with it the settings page the Delete
	// button that emitted this lives on.
	connect(
		ch, &AcqChannel::removeRequested, this, [this, ch]() { removeChannel(ch); }, Qt::QueuedConnection);

	// After attach(): the settings page binds to the channel's plot item and its axis pair,
	// both of which attach() creates.
	m_rail->registerChannel(ch);

	// Greyed from the start if the key is only declared, not written — otherwise a channel
	// created before the first cycle would look live while reading nothing.
	ch->setKeyPresent(m_store->contains(yKey));

	Q_EMIT channelAdded(ch);
	// After attach(), which is what created the channel's axis pair — the axis the marker
	// rides is made by a channel, so this and not plotAdded is the event that matters.
	retargetTriggerHandle();
	markDirty();
	return ch;
}

void AcqPlotManager::removeChannel(AcqChannel *ch)
{
	if(!ch || !m_channels.removeOne(ch)) {
		return;
	}
	Q_EMIT channelRemoved(ch);

	// Before detach(), which nulls the channel's owner: the rail row was nested under the
	// plot's row, and the rail asks the channel which plot that was.
	m_rail->unregisterChannel(ch);

	if(AcqPlot *owner = ch->plotOwner()) {
		owner->removeChannelRef(ch);
	}

	// Detach the visual now rather than leaving it to the destructor: a pull() between
	// here and the delete would be harmless (the channel is off m_channels), but a pending
	// replot() would still paint an item that is about to go. This is also what returns
	// its axis pair to the plot's pool.
	ch->detach();

	// deleteLater, not delete: this is public, and a caller may well be inside a signal
	// emitted by the very channel it is removing. The destructor detaches (idempotently)
	// and releases the depth claim.
	ch->deleteLater();

	// After detach(), which returned its axes to the plot's pool: resolving any earlier would
	// walk a list that still holds this channel and could park the marker on its axis.
	retargetTriggerHandle();
	markDirty();
}

AcqChannel *AcqPlotManager::drawOnFirstPlot(scopy::acq::ReprKind kind, const scopy::acq::DataKey &yKey,
					    const scopy::acq::DataKey &xKey)
{
	if(m_plots.isEmpty()) {
		return nullptr;
	}
	for(AcqChannel *ch : std::as_const(m_channels)) {
		if(ch && ch->key() == yKey) {
			return nullptr;
		}
	}
	return addChannel(m_plots.first(), kind, yKey, xKey);
}

void AcqPlotManager::removeChannelsFor(const QList<scopy::acq::DataKey> &keys)
{
	// Over a copy: removeChannel mutates m_channels.
	const QList<AcqChannel *> chans = m_channels;
	for(AcqChannel *ch : chans) {
		if(ch && keys.contains(ch->key())) {
			removeChannel(ch);
		}
	}
}

// --- trigger marker ---------------------------------------------------------

void AcqPlotManager::setTriggerMarker(scopy::acq::TriggerMarker *marker)
{
	m_trigMarkerView->setMarker(marker);
	retargetTriggerHandle();
}

void AcqPlotManager::setTriggerAxisKey(const scopy::acq::DataKey &key)
{
	m_trigMarkerView->setAxisKey(key);
	retargetTriggerHandle();
}

void AcqPlotManager::retargetTriggerHandle() { m_trigMarkerView->retarget(m_plots, m_plotSize); }

void AcqPlotManager::updateTriggerMarkerWindow() { m_trigMarkerView->updateWindow(m_plotSize); }

// --- source pickers --------------------------------------------------------

void AcqPlotManager::populateKeyCombo(MenuCombo *combo, bool withSampleIndex) const
{
	m_rail->populateKeyCombo(combo, withSampleIndex);
}

scopy::acq::DataKey AcqPlotManager::keyFromCombo(const MenuCombo *combo) { return AcqPlotRail::keyFromCombo(combo); }

// --- geometry --------------------------------------------------------------

int AcqPlotManager::plotSizeFor(AcqChannel *ch) const
{
	AcqPlot *p = ch ? ch->plotOwner() : nullptr;
	// The plot's own width: plots can be different widths, so a channel's width is its
	// plot's. The manager's default covers a channel read while detached, which pull()
	// should never be doing but must not crash on.
	return p ? p->plotSize() : m_plotSize;
}

void AcqPlotManager::announceMaxWindowSize()
{
	// Max, not the manager's default: two plots can be different widths, and the one ramp
	// has to satisfy the widest. A narrower plot reads its tail.
	int n = m_plotSize;
	for(AcqPlot *p : std::as_const(m_plots)) {
		n = qMax(n, p->plotSize());
	}
	Q_EMIT maxWindowSizeChanged(n);
}

void AcqPlotManager::applyWindowToIndexAxes()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		AcqAxis *ax = ch ? ch->xAxis() : nullptr;
		// Stream sources are the acquired data's own range — only the data can say
		// what they cover, so they wait for the next read.
		if(!ax || !ax->source().isIndexBased()) {
			continue;
		}
		// Same formula as AcqCurveChannel::readData(): 0..(width-1), scaled by the
		// rate for Time. The width this channel's own plot was given, which is not
		// necessarily m_plotSize — plots can be different widths.
		const int width = qMax(1, plotSizeFor(ch));
		const double rate = ch->sampleRate() > 0.0 ? ch->sampleRate() : 1.0;
		const double scale = ax->mode() == AcqAxis::Source::Mode::Time ? 1.0 / rate : 1.0;
		// requestInterval() drops repeats and is a no-op while autoscale is on, so this
		// cannot fight the autoscaler or undo the reader's zoom.
		ax->requestInterval(0.0, static_cast<double>(width - 1) * scale);
	}
}

void AcqPlotManager::setPlotSize(int n)
{
	n = qMax(1, n);
	if(n == m_plotSize) {
		return;
	}
	m_plotSize = n;

	for(AcqPlot *p : std::as_const(m_plots)) {
		p->setPlotSize(n);
	}
	// The index-based axes are the plot window by definition, so the new width *is*
	// their new range and it is known without reading a sample. Pushed here rather
	// than left to the next readData(): while stopped there is no next read, so the
	// axis kept the old width's range until the run was restarted.
	applyWindowToIndexAxes();
	reclaimAll();
	announceMaxWindowSize();
	// The marker's axis needs nothing — the curve re-requests its range on the next read —
	// but its window does, and one call carries the new width to the marker and to whoever
	// owns the processor, so the two cannot disagree.
	updateTriggerMarkerWindow();
	Q_EMIT plotSizeChanged(n);
	markDirty();
}

void AcqPlotManager::setFallbackSampleRate(double sr)
{
	if(sr <= 0.0) {
		return;
	}
	m_fallbackSampleRate = sr;
	// Also to the channels that already exist, so the controller may call this before or
	// after any of them is created without the result differing. Each channel ignores it if
	// its own producer declared a rate.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->setFallbackSampleRate(sr);
	}
}

void AcqPlotManager::reclaim(AcqChannel *ch)
{
	if(ch) {
		ch->reclaimDepth(plotSizeFor(ch));
	}
}

void AcqPlotManager::reclaimAll()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		reclaim(ch);
	}
}

// --- pull loop -------------------------------------------------------------

void AcqPlotManager::onKeysChanged(QList<scopy::acq::DataKey> keys)
{
	// Unconditionally, not a diff: DataStore::reset() and remove() erase m_claims
	// wholesale, so after either every channel's claim is gone and must be re-registered.
	reclaimAll();

	// Grey the channels whose key has gone and un-grey the ones that came back. No channel
	// is created or destroyed here — what is drawn was decided by explicit addChannel()
	// calls, and a key vanishing between runs must not take a channel's settings, rail row
	// and menu page with it.
	const QSet<scopy::acq::DataKey> live(keys.begin(), keys.end());
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->setKeyPresent(live.contains(ch->key()));
		// A stream that appeared after a settings page was built must still be selectable on
		// it, on both sides.
		ch->refreshAxisSourceChoices();
	}

	// And on every plot's add-channel section, for the same reason.
	m_rail->refreshKeyCombos();

	Q_EMIT keysAvailable(keys);
}

void AcqPlotManager::onCycleComplete()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->pull(plotSizeFor(ch));
	}
	// Deliberately no replot here — the frame timer owns repainting.
	markDirty();
}

void AcqPlotManager::onTriggerFired(const QMap<QString, scopy::acq::SampleVariant> &snap)
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->pull(plotSizeFor(ch), &snap);
	}
	markDirty();
	// Only while stopped: the trigger's single-shot stops the engine on the fire, so a Single
	// ends with a read that lands after the stop's final flush and would never be painted.
	// While running the frame timer owns the repaint, capping it at the frame rate.
	if(!m_frameTimer->isActive()) {
		flushIfDirty();
	}
}

void AcqPlotManager::onStarted()
{
	// run()/single() clear the store's chunks but keep the claims, and clearing chunks does
	// not blank a curve: it would keep drawing the previous run's tail until the first new
	// cycle landed.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->reset();
		ch->onStarted();
	}
	markDirty();
	m_frameTimer->start();
}

void AcqPlotManager::onStopped()
{
	m_frameTimer->stop();
	// Before the final flush: this is where each channel's axes take their last autoscale
	// pass, so the frame that lands is drawn against the scale the data actually ended on.
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->onStopped();
	}
	// One final flush, so the last cycle before the stop is not left unpainted.
	flushIfDirty();
}

void AcqPlotManager::flushIfDirty()
{
	if(!m_dirty) {
		return;
	}
	m_dirty = false;
	replot();
}

void AcqPlotManager::replot()
{
	for(AcqPlot *p : std::as_const(m_plots)) {
		p->replot();
	}
	Q_EMIT newData();
}

#include "moc_acqplotmanager.cpp"
