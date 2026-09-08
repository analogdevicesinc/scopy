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

#include <core/acq_engine/DataKeyCombo.h>
#include <core/acq_engine/DataStore.h>

// The factories, needed here and not in the header: they are static free functions, so
// the translation unit that calls one must include it (see the ODR note in
// gui/docking/docksettings.h).
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menusectionwidget.h>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(CAT_ACQ_PLOTMANAGER, "AcqPlotManager")

using namespace scopy;
using namespace scopy::adc;

namespace {

// The representations the reader may pick in the ADD CHANNEL section, in the order
// they are offered. ReprKind::Hidden is deliberately absent: it is the producer saying
// it does not want the stream drawn at all, so it is not a choice a plot offers.
const QList<scopy::acq::ReprKind> &pickableReprs()
{
	static const QList<scopy::acq::ReprKind> kinds{scopy::acq::ReprKind::Curve, scopy::acq::ReprKind::Digital,
						       scopy::acq::ReprKind::Annotations,
						       scopy::acq::ReprKind::Waterfall};
	return kinds;
}

} // namespace

AcqPlotManager::AcqPlotManager(scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
			       InstrumentTemplate *shell, QWidget *parent)
	: QWidget(parent)
	, m_kFrameIntervalMs(16)
	, m_store(store)
	, m_engine(engine)
	, m_shell(shell)
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

	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(m_kFrameIntervalMs);
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
	registerPlotRail(p);
	m_plotRails[p].dock = dock;

	// The title bar is the only place the plot's name shows outside the rail, so it
	// follows a rename the way the FFT waterfall's does
	// (src/freq/fftplotcomponent.cpp:128-130).
	connect(p, &AcqPlot::nameChanged, this, [dock](const QString &n) { dock->setTitle(n); });

	announceMaxWindowSize();
	Q_EMIT plotAdded(uuid);
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

	// Before unregisterPlotRail erases the record it is stored in.
	DockWrapperInterface *dock = m_plotRails.value(p).dock;

	// After the channels, so their rows are unregistered from a container that still
	// exists.
	unregisterPlotRail(p);
	m_plots.removeOne(p);

	// Nothing to do about the cursors here: they belong to the plot now, as children of
	// the AcqPlot, and go when it does. The settings page goes with the plot's menu page,
	// which unregisterPlotRail already took down.

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
	// After the dock's deleteLater and not before: the pool's axes are children of this
	// object, and deleting it first would destroy axes the widget still lists.
	delete p;

	announceMaxWindowSize();
	m_dirty = true;
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
		m_dirty = true;
	});

	// Queued: the handler destroys this channel, and with it the settings page the Delete
	// button that emitted this lives on.
	connect(ch, &AcqChannel::removeRequested, this, [this, ch]() { removeChannel(ch); }, Qt::QueuedConnection);

	// After attach(): the settings page binds to the channel's plot item and its axis pair,
	// both of which attach() creates.
	registerRail(ch);

	// Greyed from the start if the key is only declared, not written — otherwise a channel
	// created before the first cycle would look live while reading nothing.
	ch->setKeyPresent(m_store->contains(yKey));

	Q_EMIT channelAdded(ch);
	m_dirty = true;
	return ch;
}

void AcqPlotManager::removeChannel(AcqChannel *ch)
{
	if(!ch || !m_channels.removeOne(ch)) {
		return;
	}
	Q_EMIT channelRemoved(ch);

	// Before detach(), which nulls the channel's owner: the rail row was nested under the
	// plot's row, and unregisterRail asks the channel which plot that was.
	unregisterRail(ch);

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

	m_dirty = true;
}

// --- rail: the "Plots" group ------------------------------------------------

MenuSectionCollapseWidget *AcqPlotManager::railGroup()
{
	if(m_railGroup || m_shell.isNull()) {
		return m_railGroup;
	}
	m_railGroup = m_shell->addChannelGroup(tr("Plots"));
	// First, and permanently first: rows are appended, so an add-plot row created now
	// stays above every plot added later. Pinning it to the bottom instead would mean
	// re-adding it on every addPlot, which fights the rail API for nothing.
	createAddPlotRow(m_railGroup);
	return m_railGroup;
}

void AcqPlotManager::createAddPlotRow(MenuSectionCollapseWidget *group)
{
	if(m_shell.isNull() || !group) {
		return;
	}

	const QString id = QStringLiteral("acqplot:add");
	MenuControlButton *row = m_shell->addChannelRow(group, tr("+ Add plot"), QColor(), id);
	// Its checkbox would read as "this plot is selected" on a row that is not a plot. The
	// row still joins the exclusive group — that is what shows its page.
	row->checkBox()->setVisible(false);

	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	// SO_VIEW, like everything else the manager builds: adding a plot changes what is
	// drawn, never what the engine produces.
	MenuSectionCollapseWidget *section = m_shell->createMenuSection(tr("NEW PLOT"), SO_VIEW, page);

	MenuLineEdit *nameEdit = new MenuLineEdit(section);
	nameEdit->edit()->setPlaceholderText(tr("plot name"));
	section->add(nameEdit);

	MenuCombo *kindCombo = new MenuCombo(tr("Kind"), section);
	// From allPlotKinds(), so a third plot kind appears here with no edit to this file.
	const QList<AcqPlotKind> kinds = allPlotKinds();
	for(AcqPlotKind k : kinds) {
		kindCombo->combo()->addItem(plotKindName(k), static_cast<int>(k));
	}
	section->add(kindCombo);

	QPushButton *createBtn = new QPushButton(tr("Create"), section);
	Style::setStyle(createBtn, style::properties::button::borderButton);
	connect(createBtn, &QAbstractButton::clicked, this, [this, nameEdit, kindCombo]() {
		if(kindCombo->combo()->count() == 0) {
			return;
		}
		const AcqPlotKind kind = static_cast<AcqPlotKind>(kindCombo->combo()->currentData().toInt());
		QString name = nameEdit->edit()->text().trimmed();
		if(name.isEmpty()) {
			// Named after the uuid it is about to get, so two unnamed plots are still
			// distinguishable in the rail.
			name = tr("Plot %1").arg(m_plotUuidCounter);
		}
		addPlot(name, kind);
		nameEdit->edit()->clear();
	});
	section->add(createBtn);

	lay->addWidget(section);
	lay->addStretch();
	m_shell->addMenuPage(id, page);
}

void AcqPlotManager::registerPlotRail(AcqPlot *p)
{
	MenuSectionCollapseWidget *group = railGroup();
	if(!p || !group || m_plotRails.contains(p)) {
		return;
	}

	PlotRail rail;
	// Expandable, because this row is the container its channels' rows nest under — which
	// is what makes the rail the same tree as the object graph.
	rail.row = m_shell->addExpandableChannelRow(group, p->name(), QColor(), p->menuId());
	m_shell->addMenuPage(p->menuId(), createPlotPage(p, rail));
	// After createPlotPage, which is what fills in the two combo pointers.
	m_plotRails.insert(p, rail);

	if(MenuControlButton *hdr = rail.row->getControlBtn()) {
		connect(p, &AcqPlot::nameChanged, hdr, [hdr](const QString &n) { hdr->setName(n); });
	}
}

void AcqPlotManager::unregisterPlotRail(AcqPlot *p)
{
	if(!p) {
		return;
	}
	const PlotRail rail = m_plotRails.take(p);
	if(m_shell.isNull() || !rail.row) {
		return;
	}
	// The group holds the expandable row, not the header button inside it — that is what
	// addExpandableChannelRow inserted — so the layout removal is done here and
	// removeChannelRow is passed no group. It still does the other steps, and the
	// button-group one is the one that must not be skipped: QButtonGroup does not observe
	// its buttons' deletion through this path.
	if(m_railGroup) {
		m_railGroup->remove(rail.row);
	}
	m_shell->removeChannelRow(nullptr, rail.row->getControlBtn(), p->menuId(), /*deletePage=*/true);
	// deleteLater for the same reason removeChannelRow uses it for the button: this can be
	// reached from a click on the row's own page.
	rail.row->deleteLater();
}

QWidget *AcqPlotManager::createViewSection(AcqPlot *p, QWidget *parent)
{
	MenuSectionCollapseWidget *section = m_shell->createMenuSection(tr("VIEW"), SO_VIEW, parent);

	// All three are per plot, so all three are on the plot's own page rather than on a
	// bottom-rail button shared by every plot — which is what the cursors used to be.
	MenuOnOffSwitch *labelsSw = new MenuOnOffSwitch(tr("Show plot labels"), section);
	labelsSw->onOffswitch()->setChecked(p->showLabels());
	connect(labelsSw->onOffswitch(), &QAbstractButton::toggled, p, [p](bool on) { p->setShowLabels(on); });
	section->add(labelsSw);

	MenuOnOffSwitch *legendSw = new MenuOnOffSwitch(tr("Show legend"), section);
	legendSw->onOffswitch()->setChecked(p->showLegend());
	connect(legendSw->onOffswitch(), &QAbstractButton::toggled, p, [p](bool on) { p->setShowLegend(on); });
	section->add(legendSw);

	MenuOnOffSwitch *cursorSw = new MenuOnOffSwitch(tr("Show cursors"), section);
	cursorSw->onOffswitch()->setChecked(p->showCursors());
	section->add(cursorSw);

	// The cursor controls drop down under the switch rather than living in a hover off a
	// bottom-rail button. Built on first toggle, not here: CursorSettings brings a
	// CursorController with it, which installs four draggable handles and a readout
	// overlay on the canvas, and a plot whose cursors are never asked for should not pay
	// for that. Parented into the section, so it is freed with the page.
	connect(cursorSw->onOffswitch(), &QAbstractButton::toggled, p, [this, p, section](bool on) {
		p->setShowCursors(on);
		if(CursorSettings *cs = p->cursorSettings(section)) {
			// add() only on the first build — the section is a CompositeWidget and
			// would otherwise hold the same widget twice.
			if(cs->parentWidget() == section && !cs->property("acqAdded").toBool()) {
				cs->setProperty("acqAdded", true);
				// Its own setFixedWidth(200) would leave it narrower than the
				// rail and off-centre against the switches above it.
				cs->setFixedWidth(QWIDGETSIZE_MAX);
				cs->setMinimumWidth(0);
				section->add(cs);
			}
			cs->setVisible(on);
		}
	});

	return section;
}

QWidget *AcqPlotManager::createPlotPage(AcqPlot *p, PlotRail &rail)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *plotSection = m_shell->createMenuSection(tr("PLOT"), SO_VIEW, page);
	MenuLineEdit *nameEdit = new MenuLineEdit(plotSection);
	nameEdit->edit()->setText(p->name());
	connect(nameEdit->edit(), &QLineEdit::editingFinished, p,
		[p, nameEdit]() { p->setName(nameEdit->edit()->text()); });
	plotSection->add(nameEdit);
	QLabel *kindLabel = new QLabel(plotKindName(p->kind()), plotSection);
	// A readout, not a picker: the kind decides which widget class was built, and Qwt gives
	// no way to turn one plot widget into another. Changing it means a new plot.
	kindLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	plotSection->add(kindLabel);
	lay->addWidget(plotSection);

	lay->addWidget(createViewSection(p, page));

	MenuSectionCollapseWidget *addSection = m_shell->createMenuSection(tr("ADD CHANNEL"), SO_VIEW, page);

	MenuCombo *yCombo = new MenuCombo(tr("Y source"), addSection);
	populateKeyCombo(yCombo, /*withSampleIndex=*/false);
	addSection->add(yCombo);

	MenuCombo *xCombo = new MenuCombo(tr("X source"), addSection);
	populateKeyCombo(xCombo, /*withSampleIndex=*/true);
	addSection->add(xCombo);

	MenuCombo *reprCombo = new MenuCombo(tr("Draw as"), addSection);
	// Every representation, whatever kind of plot this is. A waterfall on a basic plot or
	// a curve on a spectrogram draws nothing useful, and that is the reader's business to
	// see rather than this combo's to prevent.
	for(scopy::acq::ReprKind k : pickableReprs()) {
		reprCombo->combo()->addItem(QString::fromLatin1(scopy::acq::reprKindName(k)), static_cast<int>(k));
	}
	addSection->add(reprCombo);

	QPushButton *addBtn = new QPushButton(tr("Add channel"), addSection);
	Style::setStyle(addBtn, style::properties::button::borderButton);
	connect(addBtn, &QAbstractButton::clicked, this, [this, p, yCombo, xCombo, reprCombo]() {
		const scopy::acq::DataKey yKey = keyFromCombo(yCombo);
		if(yKey.key.isEmpty()) {
			// Either nothing is selected or the store has no streams yet. The sample index is
			// not offered on the Y picker, so an empty key here is always "no source".
			qWarning(CAT_ACQ_PLOTMANAGER) << "no Y source selected";
			return;
		}
		const scopy::acq::ReprKind kind =
			static_cast<scopy::acq::ReprKind>(reprCombo->combo()->currentData().toInt());
		addChannel(p, kind, yKey, keyFromCombo(xCombo));
	});
	addSection->add(addBtn);

	lay->addWidget(addSection);

	QPushButton *delBtn = new QPushButton(tr("Delete plot"), page);
	Style::setStyle(delBtn, style::properties::button::borderButton);
	// Queued: the handler destroys this plot, and with it this page and this button, so a
	// direct connection would return into freed memory. The uuid rather than the pointer
	// for the same reason.
	connect(delBtn, &QAbstractButton::clicked, this, [this, uuid = p->uuid()]() { removePlot(uuid); },
		Qt::QueuedConnection);
	lay->addWidget(delBtn);

	lay->addStretch();

	// Kept so onKeysChanged can repopulate them in place: a stream that appears after this
	// page was built must be addable without rebuilding the page.
	rail.yCombo = yCombo;
	rail.xCombo = xCombo;

	return page;
}

// --- rail: channel rows ----------------------------------------------------

void AcqPlotManager::registerRail(AcqChannel *ch)
{
	if(!ch || m_shell.isNull() || m_railRows.contains(ch)) {
		return;
	}
	// Under its plot's row, not under the group: the rail is the same tree as the object
	// graph. A channel whose plot has no rail entry gets no row rather than a row in the
	// wrong place.
	AcqPlot *owner = ch->plotOwner();
	if(!owner || !m_plotRails.contains(owner)) {
		return;
	}
	CollapsableMenuControlButton *container = m_plotRails[owner].row;
	if(!container) {
		return;
	}

	const QString id = ch->menuId();
	MenuControlButton *row = m_shell->addChannelSwitchRow(container, ch->name(), ch->color(), id);
	m_railRows.insert(ch, row);

	// Built once, here, and never rebuilt — which is only sound because the key and the
	// kind are fixed for the channel's life. The stack owns it from here: unregisterRail
	// asks removeChannelRow to delete it.
	m_shell->addMenuPage(id, ch->createSettingsPage(m_shell.data()));

	if(SmallOnOffSwitch *sw = InstrumentTemplate::rowSwitch(row)) {
		connect(sw, &QAbstractButton::toggled, ch, &AcqChannel::setEnabled);
		// Both directions: a channel can be disabled from code (a vanished key), and the
		// switch has to show it.
		connect(ch, &AcqChannel::enabledChanged, sw, [sw](bool en) {
			QSignalBlocker b(sw);
			sw->setChecked(en);
		});
		sw->setChecked(ch->isEnabled());

		// A vanished key greys the row rather than removing it. Disabling the switch is the
		// whole of "greyed": the channel is already force-disabled underneath, and leaving
		// the switch live would let the reader tick a channel that cannot read.
		connect(ch, &AcqChannel::keyPresentChanged, sw, [sw](bool present) { sw->setEnabled(present); });
		sw->setEnabled(ch->isKeyPresent());
	}

	// The rail row is the channel's name in the UI, so a rename from the settings page has
	// to reach it.
	connect(ch, &AcqChannel::nameChanged, row, [row](const QString &n) { row->setName(n); });
	connect(ch, &AcqChannel::colorChanged, row, [row](const QColor &c) { row->setColor(c); });
}

void AcqPlotManager::unregisterRail(AcqChannel *ch)
{
	if(!ch) {
		return;
	}
	MenuControlButton *row = m_railRows.take(ch);
	if(m_shell.isNull() || !row) {
		return;
	}
	// The container is the plot's row, which is where addChannelSwitchRow put it. Null when
	// the plot's rail entry has already gone — removeChannelRow tolerates that and still
	// unregisters the button and its page, which is the part that must not be skipped.
	AcqPlot *owner = ch->plotOwner();
	CompositeWidget *container = nullptr;
	if(owner && m_plotRails.contains(owner)) {
		container = m_plotRails[owner].row;
	}
	// Four steps, done by the shell: button group, layout, menu page, deleteLater.
	// deletePage=true because nothing here keeps the page — it was built once for this
	// channel and dies with it.
	m_shell->removeChannelRow(container, row, ch->menuId(), /*deletePage=*/true);
}

// --- source pickers --------------------------------------------------------

void AcqPlotManager::populateKeyCombo(MenuCombo *combo, bool withSampleIndex) const
{
	if(!combo) {
		return;
	}
	// The declared ∪ written / sort / preserve-selection logic lives in core, shared with
	// every other key picker in the stack; see DataKeyCombo.h for why each step is as it is.
	scopy::acq::populateKeyCombo(combo->combo(), m_store, m_engine, withSampleIndex);
}

scopy::acq::DataKey AcqPlotManager::keyFromCombo(const MenuCombo *combo)
{
	if(!combo) {
		return scopy::acq::DataKey();
	}
	// const_cast because MenuCombo::combo() is non-const; nothing here mutates it.
	return scopy::acq::keyFromCombo(const_cast<MenuCombo *>(combo)->combo());
}

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
	// Nothing to set on the sample-index axes here either — see addChannel().
	reclaimAll();
	announceMaxWindowSize();
	m_dirty = true;
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
	for(auto it = m_plotRails.begin(); it != m_plotRails.end(); ++it) {
		populateKeyCombo(it->yCombo.data(), /*withSampleIndex=*/false);
		populateKeyCombo(it->xCombo.data(), /*withSampleIndex=*/true);
	}

	Q_EMIT keysAvailable(keys);
}

void AcqPlotManager::onCycleComplete()
{
	for(AcqChannel *ch : std::as_const(m_channels)) {
		ch->pull(plotSizeFor(ch));
	}
	// Deliberately no replot here — the frame timer owns repainting.
	m_dirty = true;
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
	m_dirty = true;
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
	if(m_dirty) {
		m_dirty = false;
		replot();
	}
}

void AcqPlotManager::replot()
{
	for(AcqPlot *p : std::as_const(m_plots)) {
		p->replot();
	}
	Q_EMIT newData();
}

#include "moc_acqplotmanager.cpp"
