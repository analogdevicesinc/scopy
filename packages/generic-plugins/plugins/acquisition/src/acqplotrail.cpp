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

#include "acqplotrail.h"

#include "acqchannel.h"
#include "acqplot.h"

#include <core/acq_engine/datakeycombo.h>
#include <core/acq_engine/datastore.h>

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
#include <QSignalBlocker>
#include <QVBoxLayout>

Q_LOGGING_CATEGORY(CAT_ACQ_PLOTRAIL, "AcqPlotRail")

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

AcqPlotRail::AcqPlotRail(InstrumentTemplate *shell, scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
			 QObject *parent)
	: QObject(parent)
	, m_shell(shell)
	, m_store(store)
	, m_engine(engine)
{
	if(m_shell.isNull()) {
		return;
	}
	m_group = m_shell->addChannelGroup(tr("Plots"));
	// First, and permanently first: rows are appended, so an add-plot row created now
	// stays above every plot added later. Pinning it to the bottom instead would mean
	// re-adding it on every plot, which fights the rail API for nothing.
	createAddPlotRow();
}

// --- the add-plot row -------------------------------------------------------

void AcqPlotRail::createAddPlotRow()
{
	const QString id = QStringLiteral("acqplot:add");
	// A button rather than a channel row: it names no plot, so a row's colour swatch,
	// switch and gear would all be furniture for something that is not there. It still
	// joins the rail's exclusive group, which is what shows its page and un-checks it
	// again when a real plot is selected.
	m_shell->addRailActionButton(m_group, tr("Add plot"), id);

	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	// SO_VIEW, like everything else here: adding a plot changes what is drawn, never what
	// the engine produces.
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
		// The name as typed, blank included: only the manager knows which uuid is next, so
		// naming an unnamed plot is its call.
		Q_EMIT addPlotRequested(nameEdit->edit()->text().trimmed(), kind);
		nameEdit->edit()->clear();
	});
	section->add(createBtn);

	lay->addWidget(section);
	lay->addStretch();
	m_shell->addMenuPage(id, page);
}

// --- plot rows --------------------------------------------------------------

void AcqPlotRail::registerPlot(AcqPlot *p)
{
	if(!p || m_shell.isNull() || !m_group || m_entries.contains(p)) {
		return;
	}

	Entry entry;
	// Expandable, because this row is the container its channels' rows nest under — which
	// is what makes the rail the same tree as the object graph.
	entry.row = m_shell->addExpandableChannelRow(m_group, p->name(), QColor(), p->menuId());
	m_shell->addMenuPage(p->menuId(), createPlotPage(p, entry));
	// After createPlotPage, which is what fills in the two combo pointers.
	m_entries.insert(p, entry);

	if(MenuControlButton *hdr = entry.row->getControlBtn()) {
		connect(p, &AcqPlot::nameChanged, hdr, [hdr](const QString &n) { hdr->setName(n); });
	}
}

void AcqPlotRail::unregisterPlot(AcqPlot *p)
{
	if(!p) {
		return;
	}
	const Entry entry = m_entries.take(p);
	if(m_shell.isNull() || !entry.row) {
		return;
	}
	// The group holds the expandable row, not the header button inside it — that is what
	// addExpandableChannelRow inserted — so the layout removal is done here and
	// removeChannelRow is passed no group. It still does the other steps, and the
	// button-group one is the one that must not be skipped: QButtonGroup does not observe
	// its buttons' deletion through this path.
	if(m_group) {
		m_group->remove(entry.row);
	}
	m_shell->removeChannelRow(nullptr, entry.row->getControlBtn(), p->menuId(), /*deletePage=*/true);
	// deleteLater for the same reason removeChannelRow uses it for the button: this can be
	// reached from a click on the row's own page.
	entry.row->deleteLater();
}

QWidget *AcqPlotRail::createViewSection(AcqPlot *p, QWidget *parent)
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
	connect(cursorSw->onOffswitch(), &QAbstractButton::toggled, p, [p, section](bool on) {
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

QWidget *AcqPlotRail::createPlotPage(AcqPlot *p, Entry &entry)
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
			qWarning(CAT_ACQ_PLOTRAIL) << "no Y source selected";
			return;
		}
		const scopy::acq::ReprKind kind =
			static_cast<scopy::acq::ReprKind>(reprCombo->combo()->currentData().toInt());
		Q_EMIT addChannelRequested(p, kind, yKey, keyFromCombo(xCombo));
	});
	addSection->add(addBtn);

	lay->addWidget(addSection);

	QPushButton *delBtn = new QPushButton(tr("Delete plot"), page);
	Style::setStyle(delBtn, style::properties::button::borderButton);
	// Queued: the handler destroys this plot, and with it this page and this button, so a
	// direct connection would return into freed memory. The uuid rather than the pointer
	// for the same reason.
	connect(
		delBtn, &QAbstractButton::clicked, this,
		[this, uuid = p->uuid()]() { Q_EMIT removePlotRequested(uuid); }, Qt::QueuedConnection);
	lay->addWidget(delBtn);

	lay->addStretch();

	// Kept so refreshKeyCombos can repopulate them in place: a stream that appears after this
	// page was built must be addable without rebuilding the page.
	entry.yCombo = yCombo;
	entry.xCombo = xCombo;

	return page;
}

// --- channel rows -----------------------------------------------------------

void AcqPlotRail::registerChannel(AcqChannel *ch)
{
	if(!ch || m_shell.isNull() || m_channelRows.contains(ch)) {
		return;
	}
	// Under its plot's row, not under the group: the rail is the same tree as the object
	// graph. A channel whose plot has no rail entry gets no row rather than a row in the
	// wrong place.
	AcqPlot *owner = ch->plotOwner();
	if(!owner || !m_entries.contains(owner)) {
		return;
	}
	CollapsableMenuControlButton *container = m_entries[owner].row;
	if(!container) {
		return;
	}

	const QString id = ch->menuId();
	MenuControlButton *row = m_shell->addChannelSwitchRow(container, ch->name(), ch->color(), id);
	m_channelRows.insert(ch, row);

	// Built once, here, and never rebuilt — which is only sound because the key and the
	// kind are fixed for the channel's life. The stack owns it from here: unregisterChannel
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

void AcqPlotRail::unregisterChannel(AcqChannel *ch)
{
	if(!ch) {
		return;
	}
	MenuControlButton *row = m_channelRows.take(ch);
	if(m_shell.isNull() || !row) {
		return;
	}
	// The container is the plot's row, which is where addChannelSwitchRow put it. Null when
	// the plot's rail entry has already gone — removeChannelRow tolerates that and still
	// unregisters the button and its page, which is the part that must not be skipped.
	AcqPlot *owner = ch->plotOwner();
	CompositeWidget *container = nullptr;
	if(owner && m_entries.contains(owner)) {
		container = m_entries[owner].row;
	}
	// Four steps, done by the shell: button group, layout, menu page, deleteLater.
	// deletePage=true because nothing here keeps the page — it was built once for this
	// channel and dies with it.
	m_shell->removeChannelRow(container, row, ch->menuId(), /*deletePage=*/true);
}

// --- source pickers ---------------------------------------------------------

void AcqPlotRail::refreshKeyCombos()
{
	for(auto it = m_entries.begin(); it != m_entries.end(); ++it) {
		populateKeyCombo(it->yCombo.data(), /*withSampleIndex=*/false);
		populateKeyCombo(it->xCombo.data(), /*withSampleIndex=*/true);
	}
}

void AcqPlotRail::populateKeyCombo(MenuCombo *combo, bool withSampleIndex) const
{
	if(!combo) {
		return;
	}
	// The declared ∪ written / sort / preserve-selection logic lives in core, shared with
	// every other key picker in the stack; see datakeycombo.h for why each step is as it is.
	scopy::acq::populateKeyCombo(combo->combo(), m_store, m_engine, withSampleIndex);
}

scopy::acq::DataKey AcqPlotRail::keyFromCombo(const MenuCombo *combo)
{
	if(!combo) {
		return scopy::acq::DataKey();
	}
	// const_cast because MenuCombo::combo() is non-const; nothing here mutates it.
	return scopy::acq::keyFromCombo(const_cast<MenuCombo *>(combo)->combo());
}

#include "moc_acqplotrail.cpp"
