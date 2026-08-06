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

#include "instrumenttemplate.h"

#include "menu_anim.hpp"
#include "style.h"
#include "widgets/menucollapsesection.h"
#include "widgets/toolbuttons.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSplitter>
#include <QVBoxLayout>

using namespace scopy;

// Region sizes. Widths/heights are px in the ToolTemplate API, so they come from the
// unit_* ladder rather than from literals. These land on the sizes the existing
// instruments use by hand — a ~210px rail and a ~300px menu.
static int channelRailWidth()
{
	return Style::getDimension(json::global::unit_6) + Style::getDimension(json::global::unit_5);
}
static int menuWidth()
{
	return Style::getDimension(json::global::unit_6) * 2 + Style::getDimension(json::global::unit_4);
}
// The debug panel is docked, so this is plot height it takes away permanently while open.
// ~256px: enough for a tab bar plus about eight tree rows.
static int debugPanelHeight()
{
	return Style::getDimension(json::global::unit_6) * 2;
}
// Both fps fields, so the readout and the field under it line up. ~80px: wide enough
// for the "target FPS" placeholder, which is the longer of the two strings.
static int fpsFieldWidth()
{
	return Style::getDimension(json::global::unit_5);
}

DebugPopup::DebugPopup(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	int pad = Style::getDimension(json::global::unit_1);
	lay->setContentsMargins(pad, pad, pad, pad);
	setLayout(lay);

	// The panel is a surface, so it carries a fill — unlike the transparent rail above
	// it. WA_StyledBackground is what makes a plain QWidget honour a stylesheet
	// background at all; without it this widget paints nothing of its own.
	Style::setStyle(this, style::properties::widget::overlayMenu);
	setAttribute(Qt::WA_StyledBackground, true);

	// Full rail width, but only the height it asks for: Expanding vertically would make
	// the panel and the plot split the window's slack between them, so opening the
	// panel would keep stealing plot height as the window grew.
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	m_tabs = new QTabWidget(this);
	Style::setStyle(m_tabs->tabBar(), style::properties::tabwidget::smallTabItem);
	lay->addWidget(m_tabs);
}

DebugPopup::~DebugPopup() {}

void DebugPopup::addTab(QWidget *w, const QString &title) { m_tabs->addTab(w, title); }

QTabWidget *DebugPopup::tabs() const { return m_tabs; }

InstrumentTemplate::InstrumentTemplate(QWidget *parent)
	: QWidget(parent)
	, m_center(nullptr)
	, m_lastFrameStamp(0)
	, m_debugPopup(nullptr)
{
	setupRegions();
	setupSlots();
	setupTopRail();
	setupBottomRail();
}

InstrumentTemplate::~InstrumentTemplate() {}

void InstrumentTemplate::setupRegions()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	setLayout(lay);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	lay->addWidget(m_tool);

	m_tool->topContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
	m_tool->topCentral()->setVisible(false);
	m_tool->bottomCentral()->setVisible(false);
	// The right menu has its own control strip in ToolTemplate; the rails carry
	// everything here, so it stays hidden.
	m_tool->topContainerMenuControl()->setVisible(false);

	// ToolTemplate's own side containers stay unused: MenuHAnim pins minimumWidth ==
	// maximumWidth, which a splitter handle cannot move, and its open/close animation
	// would rewrite both on every menu switch and undo the user's drag. The two menus
	// live in a splitter instead — same three regions, draggable — as in
	// adcinstrument.cpp:63. Their stacks are reparented, so leftStack()/rightStack()
	// (and every addMenuPage() built on them) keep working unchanged.
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(false);

	m_splitter = new QSplitter(Qt::Horizontal, this);
	// global.qss paints every QSplitter background_subtle; that is already what
	// ToolTemplate paints, but the handles are transparent glyphs over it, so keep the
	// splitter itself out of the way and let the panels below carry the only fills.
	m_splitter->setStyleSheet(QStringLiteral("QSplitter { background-color: transparent; }"));

	m_leftPanel = new QWidget(m_splitter);
	m_rightPanel = new QWidget(m_splitter);
	for(QWidget *panel : {m_leftPanel, m_rightPanel}) {
		QVBoxLayout *panelLay = new QVBoxLayout(panel);
		panelLay->setContentsMargins(0, 0, 0, 0);
		panelLay->setSpacing(0);
		// The rail and the menu ask for a few hundred px through their stacks'
		// minimumSizeHint; a floor that high is a floor the handle cannot cross, so
		// the panels are allowed all the way down to collapsed.
		panel->setMinimumWidth(0);
		// Only the two menus get a background. ToolTemplate already paints itself
		// background_subtle, so the center inherits it by staying transparent — see
		// docs/acq-engine/08-instrument-layout.md.
		Style::setBackgroundColor(panel, json::theme::background_primary);
	}

	m_leftPanel->layout()->addWidget(m_tool->leftStack());
	m_rightPanel->layout()->addWidget(m_tool->rightStack());

	m_splitter->addWidget(m_leftPanel);
	// index 1 is left free for the center frame, which setupSlots() inserts.
	m_splitter->addWidget(m_rightPanel);

	m_tool->addWidgetToCentralContainerHelper(m_splitter);

	m_vcm = new VerticalChannelManager(m_leftPanel);
	m_tool->leftStack()->add("channelRail", m_vcm);

	m_channelGroup = new QButtonGroup(this);
	m_channelGroup->setExclusive(true);
}

void InstrumentTemplate::setupSlots()
{
	// The four slots exist from construction, so an owner never has to ask whether a
	// region is there before annotating a plot. They are plain layouts with no spacing
	// and no margins, which is what makes an unused slot collapse to zero.
	m_centerFrame = new QWidget(m_splitter);
	m_centerFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_centerCol = new QVBoxLayout(m_centerFrame);
	m_centerCol->setContentsMargins(0, 0, 0, 0);
	m_centerCol->setSpacing(0);

	QWidget *row = new QWidget(m_centerFrame);
	row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_centerRow = new QHBoxLayout(row);
	m_centerRow->setContentsMargins(0, 0, 0, 0);
	m_centerRow->setSpacing(0);

	QVBoxLayout *top = new QVBoxLayout();
	QVBoxLayout *bottom = new QVBoxLayout();
	QVBoxLayout *left = new QVBoxLayout();
	QVBoxLayout *right = new QVBoxLayout();
	for(QBoxLayout *l : {top, bottom, left, right}) {
		l->setContentsMargins(0, 0, 0, 0);
		l->setSpacing(0);
	}

	m_centerCol->addLayout(top);
	m_centerCol->addWidget(row, 1);
	m_centerCol->addLayout(bottom);

	m_centerRow->addLayout(left);
	// index 1 is where setCenterWidget() drops the PlotManager, between left and right.
	m_centerRow->addLayout(right);

	m_slots.insert(PS_TOP, top);
	m_slots.insert(PS_BOTTOM, bottom);
	m_slots.insert(PS_LEFT, left);
	m_slots.insert(PS_RIGHT, right);

	// Between the channel rail and the menu. Only the center stretches, so resizing the
	// window grows the plot and leaves both menus at the width the user dragged them to.
	m_splitter->insertWidget(1, m_centerFrame);
	m_splitter->setStretchFactor(0, 0);
	m_splitter->setStretchFactor(1, 1);
	m_splitter->setStretchFactor(2, 0);
	// The center figure only has to be larger than the slack a first show() has to
	// distribute; stretch settles the real widths on the first resize.
	m_splitter->setSizes({channelRailWidth(), 10 * menuWidth(), menuWidth()});
}

void InstrumentTemplate::setupTopRail()
{
	m_infoBtn = new InfoBtn(this);
	m_printBtn = new PrintIconBtn(this);

	// No ExportBtn exists in gui/; the rail wants an icon button, so build one the
	// same way PrintBtn does and give it the existing save glyph.
	m_exportBtn = new QPushButton(this);
	m_exportBtn->setCheckable(false);
	m_exportBtn->setIcon(
		Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/save.svg",
				 Style::getColor(json::theme::content_default)));
	m_exportBtn->setToolTip("Export");
	Style::setStyle(m_exportBtn, style::properties::button::squareIconButton);

	// Engine controls (mode · buffer · plot window) are the owner's to supply — the
	// shell only reserves the slot, so their order survives interleaved calls.
	//
	// The rail itself is transparent, so a bare row of controls floats with nothing
	// tying mode/buffer/fps together. This section is the one filled surface up here:
	// it reads as "the engine's settings" rather than as three unrelated widgets.
	// Same fill and corner as the icon buttons beside it: squareIconButton's own rule is
	// QPushButton-scoped and can't be reused here, but overlayMenu is the same
	// background_primary + radius_1 pair applied to a QWidget.
	m_engineSection = new QWidget(this);
	Style::setStyle(m_engineSection, style::properties::widget::overlayMenu);
	// overlayMenu supplies the corner, but not the fill: ToolTemplate paints itself with a
	// widget-level `.QWidget { background_subtle }`, which cascades to descendants and
	// outranks the app-wide stylesheet a Style property is matched by. `.QWidget` matches
	// exact QWidget instances only, so the buttons beside this section escape it and a plain
	// container like this one does not. Restated at widget level, where it wins; the
	// border-radius declaration is untouched and still comes from overlayMenu.
	Style::setBackgroundColor(m_engineSection, json::theme::background_primary);
	// Only the width its controls need: the rail's spacer takes the slack, so without
	// this the section stretches to the Run button and leaves a field of empty fill.
	m_engineSection->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	QHBoxLayout *sectionLay = new QHBoxLayout(m_engineSection);
	int pad = Style::getDimension(json::global::unit_0_5);
	sectionLay->setContentsMargins(pad, 0, pad, 0);
	sectionLay->setSpacing(Style::getDimension(json::global::unit_1));

	m_engineControls = new QWidget(m_engineSection);
	m_engineControlsLay = new QHBoxLayout(m_engineControls);
	m_engineControlsLay->setContentsMargins(0, 0, 0, 0);
	m_engineControlsLay->setSpacing(Style::getDimension(json::global::unit_1));

	// Measured above, target below: the two are the same quantity, one observed and
	// one asked for, so they read as a pair rather than as another engine control.
	// Stacked because the rail is 44px tall and two rows of this size fit where a
	// third labelled control beside the buffer size would not.
	QWidget *fpsBox = new QWidget(m_engineSection);
	QVBoxLayout *fpsLay = new QVBoxLayout(fpsBox);
	fpsLay->setContentsMargins(0, 0, 0, 0);
	fpsLay->setSpacing(0);

	m_fps = new QLabel(fpsBox);
	m_fps->setFixedWidth(fpsFieldWidth());
	Style::setStyle(m_fps, style::properties::label::subtle);

	m_targetFps = new QLineEdit(fpsBox);
	m_targetFps->setFixedWidth(fpsFieldWidth());
	m_targetFps->setFixedHeight(Style::getDimension(json::global::unit_2));
	m_targetFps->setPlaceholderText("target FPS");
	m_targetFps->setToolTip("Target FPS — how often the instrument is notified of new data. Empty for unlimited.");
	m_targetFps->setValidator(new QIntValidator(1, 1000, m_targetFps));
	Style::setStyle(m_targetFps, style::properties::lineedit::menuLineEdit);

	fpsLay->addWidget(m_fps);
	fpsLay->addWidget(m_targetFps);

	connect(m_targetFps, &QLineEdit::editingFinished, this, [this]() {
		bool ok = false;
		const int v = m_targetFps->text().toInt(&ok);
		// Empty or unparsable reads as "no ceiling", which is what the engine's
		// own 0 means — so clearing the field is a way to say it.
		Q_EMIT targetFpsChanged(ok && v > 0 ? v : 0);
	});

	sectionLay->addWidget(m_engineControls);
	sectionLay->addWidget(fpsBox);

	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_printBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_exportBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_engineSection, TTA_LEFT);

	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
}

void InstrumentTemplate::setupBottomRail()
{
	m_debugBtn = new QPushButton("Debug", this);
	m_debugBtn->setCheckable(true);
	Style::setStyle(m_debugBtn, style::properties::button::blueGrayButton);
	Style::setStyle(m_debugBtn, style::properties::label::menuMedium);

	m_tool->addWidgetToBottomContainerHelper(m_debugBtn, TTA_LEFT);

	connect(m_debugBtn, &QAbstractButton::toggled, this, [this](bool b) {
		if(b) {
			debugPopup()->setVisible(true);
		} else if(m_debugPopup) {
			m_debugPopup->setVisible(false);
		}
	});
}

ToolTemplate *InstrumentTemplate::tool() const { return m_tool; }

QSplitter *InstrumentTemplate::splitter() const { return m_splitter; }

void InstrumentTemplate::setCenterWidget(QWidget *w)
{
	if(m_center) {
		m_centerRow->removeWidget(m_center);
		m_center->setParent(nullptr);
	}

	m_center = w;
	if(!w) {
		return;
	}

	// Between PS_LEFT and PS_RIGHT, and the only stretching item in the row.
	m_centerRow->insertWidget(1, w, 1);
}

QWidget *InstrumentTemplate::centerWidget() const { return m_center; }

void InstrumentTemplate::addToSlot(PlotSlot s, QWidget *w) { m_slots.value(s)->addWidget(w); }

void InstrumentTemplate::removeFromSlot(PlotSlot s, QWidget *w)
{
	m_slots.value(s)->removeWidget(w);
	w->setParent(nullptr);
}

QBoxLayout *InstrumentTemplate::slot(PlotSlot s) const { return m_slots.value(s); }

void InstrumentTemplate::addToTopRail(QWidget *w, ToolTemplateAlignment a)
{
	m_tool->addWidgetToTopContainerHelper(w, a);
}

void InstrumentTemplate::addToBottomRail(QWidget *w, ToolTemplateAlignment a)
{
	m_tool->addWidgetToBottomContainerHelper(w, a);
}

RunBtn *InstrumentTemplate::runBtn() const { return m_runBtn; }
SingleShotBtn *InstrumentTemplate::singleBtn() const { return m_singleBtn; }
InfoBtn *InstrumentTemplate::infoBtn() const { return m_infoBtn; }
PrintIconBtn *InstrumentTemplate::printBtn() const { return m_printBtn; }
QPushButton *InstrumentTemplate::exportBtn() const { return m_exportBtn; }
QWidget *InstrumentTemplate::engineSection() const { return m_engineSection; }

void InstrumentTemplate::addEngineControl(QWidget *w)
{
	w->setSizePolicy(QSizePolicy::Preferred, w->sizePolicy().verticalPolicy());
	m_engineControlsLay->addWidget(w);
}

void InstrumentTemplate::setFps(qint64 timestampMs)
{
	// Same rolling average as FPSInfo (gui/src/widgets/plotinfowidgets.cpp:144), but
	// driven by the owner: FPSInfo needs a PlotWidget and the shell has none.
	const int avgSize = 10;

	if(m_lastFrameStamp == 0) {
		m_lastFrameStamp = timestampMs;
		return;
	}

	m_frameTimes.append(timestampMs - m_lastFrameStamp);
	if(m_frameTimes.size() > avgSize) {
		m_frameTimes.removeFirst();
	}
	m_lastFrameStamp = timestampMs;

	qint64 avg = 0;
	for(qint64 t : m_frameTimes) {
		avg += t;
	}
	avg /= m_frameTimes.size();
	if(avg <= 0) {
		return;
	}

	m_fps->setText(QString::number(1000. / avg, 'g', 3) + " FPS");
}

QLabel *InstrumentTemplate::fpsLabel() const { return m_fps; }

QLineEdit *InstrumentTemplate::targetFpsEdit() const { return m_targetFps; }

int InstrumentTemplate::targetFps() const
{
	bool ok = false;
	const int v = m_targetFps->text().toInt(&ok);
	return ok && v > 0 ? v : 0;
}

void InstrumentTemplate::setTargetFps(int fps)
{
	// Blocked: this reflects what the owner already applied, so re-emitting would
	// send it straight back.
	QSignalBlocker b(m_targetFps);
	m_targetFps->setText(fps > 0 ? QString::number(fps) : QString());
}

VerticalChannelManager *InstrumentTemplate::channelRail() const { return m_vcm; }

MenuSectionCollapseWidget *InstrumentTemplate::addChannelGroup(const QString &title)
{
	if(m_groups.contains(title)) {
		return m_groups.value(title);
	}

	MenuSectionCollapseWidget *group = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, m_vcm);

	// The rail is already a surface; a panel fill per group would be a second border
	// saying the same thing.
	Style::setBackgroundColor(group->menuSection(), QString("transparent"));

	m_vcm->add(group);
	m_groups.insert(title, group);
	return group;
}

void InstrumentTemplate::configureChannelRow(MenuControlButton *row, QWidget *insert, CompositeWidget *group,
					     const QString &name, const QColor &color, const QString &menuId,
					     int indent)
{
	row->setName(name);
	// Only when asked for. Colour on a rail row means "this is the curve you see in
	// that colour", so a source, processor or plot group must keep the row's neutral
	// default rather than claim a curve it does not have.
	if(color.isValid()) {
		row->setColor(color);
		row->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
	}
	row->enableToolTip(true);
	// The gear glyph replaces the real gear button: the whole row opens the page, which
	// is a larger and more forgiving target.
	row->button()->setVisible(false);
	row->setIconEnabled(true);
	row->setOpenMenuChecksThis(true);
	row->setDoubleClickToOpenMenu(false);
	// MenuControlButton defaults to Maximum, which sizes it to its label and leaves the
	// rail's width unused — rows have to fill it so the name, gear and switch line up
	// down the column instead of each row ending wherever its text does.
	insert->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	if(indent > 0) {
		// Stacking is shown by indentation — processors on top of processors. Only
		// the left margin moves; the rest is the row's own padding.
		QMargins m = row->layout()->contentsMargins();
		m.setLeft(m.left() + indent * Style::getDimension(json::global::unit_2));
		row->layout()->setContentsMargins(m);
	}

	m_channelGroup->addButton(row);
	if(group) {
		group->add(insert);
	}

	if(!menuId.isEmpty()) {
		connect(row, &QAbstractButton::clicked, this, [this, menuId](bool b) {
			if(b) {
				showMenuPage(menuId);
			}
		});
	}
}

MenuControlButton *InstrumentTemplate::makeChannelRow(CompositeWidget *group, const QString &name,
						     const QColor &color, const QString &menuId, int indent,
						     bool asSwitch)
{
	MenuControlButton *btn = new MenuControlButton();

	if(asSwitch) {
		// Plots create and destroy rather than mute, so the switch replaces the
		// checkbox. Row selection stays exclusive — it only decides which page shows.
		btn->checkBox()->setVisible(false);
		SmallOnOffSwitch *sw = new SmallOnOffSwitch(btn);
		sw->setChecked(true);
		btn->layout()->addWidget(sw);
	}

	configureChannelRow(btn, btn, group, name, color, menuId, indent);
	return btn;
}

MenuControlButton *InstrumentTemplate::addChannelRow(CompositeWidget *group, const QString &name,
						    const QColor &color, const QString &menuId, int indent)
{
	return makeChannelRow(group, name, color, menuId, indent, false);
}

MenuControlButton *InstrumentTemplate::addChannelSwitchRow(CompositeWidget *group, const QString &name,
							  const QColor &color, const QString &menuId, int indent)
{
	return makeChannelRow(group, name, color, menuId, indent, true);
}

CollapsableMenuControlButton *InstrumentTemplate::addExpandableChannelRow(CompositeWidget *group, const QString &name,
									 const QColor &color, const QString &menuId)
{
	CollapsableMenuControlButton *row = new CollapsableMenuControlButton();
	MenuControlButton *hdr = row->getControlBtn();

	// Two things the constructor did that have to be undone: it made the header
	// non-checkable, and it took the checkbox for the collapse arrow. Checkable again
	// because the header is a rail row like any other — it joins the exclusive group
	// and opens a page. The arrow keeps the checkbox, so a coloured circle is not
	// available here and `color` is only carried for the label.
	hdr->setCheckable(true);
	configureChannelRow(hdr, row, group, name, QColor(), menuId, 0);
	if(color.isValid()) {
		hdr->setColor(color);
	}

	// The arrow is inside the header button, so a click on it would also select the row
	// and switch the page. Transparent-for-mouse is not an option — it *is* the
	// collapse control — so the row body's own click is what expands instead: one
	// target, and selecting a source shows its children.
	QCheckBox *arrow = hdr->checkBox();
	arrow->setAttribute(Qt::WA_TransparentForMouseEvents);
	connect(hdr, &QAbstractButton::clicked, arrow, [arrow]() { arrow->setChecked(true); });

	return row;
}

SmallOnOffSwitch *InstrumentTemplate::rowSwitch(MenuControlButton *row)
{
	if(!row) {
		return nullptr;
	}
	// makeChannelRow appends the switch into the row's own layout rather than
	// exposing it, so this is the only way back to it.
	return row->findChild<SmallOnOffSwitch *>();
}

void InstrumentTemplate::removeChannelRow(CompositeWidget *group, MenuControlButton *row, const QString &menuId)
{
	if(!row) {
		return;
	}

	// First, and not optional: QButtonGroup does not observe its buttons' deletion
	// through this path, so a row left in the group is a dangling pointer the next
	// exclusive-selection walk would follow.
	m_channelGroup->removeButton(row);

	if(group) {
		// Layout only — CompositeWidget::remove neither deletes nor unregisters.
		group->remove(row);
	}
	if(!menuId.isEmpty()) {
		removeMenuPage(menuId);
	}

	// deleteLater, not delete: this is usually called from the row's own toggled
	// handler, so the button must outlive the signal emission.
	row->deleteLater();
}

MenuControlButton *InstrumentTemplate::addRailHeaderRow(const QString &name, const QString &menuId)
{
	MenuControlButton *btn = new MenuControlButton(m_vcm);
	btn->setName(name);
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(false);
	btn->checkBox()->setVisible(false);
	btn->button()->setVisible(false);
	btn->setIconEnabled(true);

	m_channelGroup->addButton(btn);
	// addTop() puts it outside the scroll area, so it stays put while rows scroll.
	m_vcm->addTop(btn);

	if(!menuId.isEmpty()) {
		connect(btn, &QAbstractButton::clicked, this, [this, menuId](bool b) {
			if(b) {
				showMenuPage(menuId);
			}
		});
	}

	return btn;
}

QButtonGroup *InstrumentTemplate::channelGroup() const { return m_channelGroup; }

void InstrumentTemplate::addMenuPage(const QString &id, QWidget *w) { m_tool->rightStack()->add(id, w); }

void InstrumentTemplate::removeMenuPage(const QString &id) { m_tool->rightStack()->remove(id); }

bool InstrumentTemplate::hasMenuPage(const QString &id) const { return m_tool->rightStack()->contains(id); }

void InstrumentTemplate::showMenuPage(const QString &id)
{
	if(!m_tool->rightStack()->contains(id)) {
		return;
	}
	m_tool->rightStack()->show(id);
	// Selecting a row has to bring the menu back if it was dragged shut, otherwise the
	// page switches behind a collapsed panel and the click looks like it did nothing.
	// Any other width is the user's, and is left alone.
	if(m_splitter->sizes().at(2) == 0) {
		QList<int> sizes = m_splitter->sizes();
		sizes[1] -= menuWidth();
		sizes[2] = menuWidth();
		m_splitter->setSizes(sizes);
	}
	Q_EMIT menuPageSelected(id);
}

QLabel *InstrumentTemplate::createOwnerPill(SettingOwner owner, QWidget *parent)
{
	QString text;
	// statusLabel's three variants are already an outlined, rounded chip in the right
	// colours — blue for free, green for a channel write, red for a shared one.
	QString variant;

	switch(owner) {
	case SO_VIEW:
		text = "view";
		variant = "true";
		break;
	case SO_CH:
		text = "ch";
		variant = "success";
		break;
	case SO_DEV:
	default:
		text = "dev";
		variant = "error";
		break;
	}

	QLabel *pill = new QLabel(text, parent);
	pill->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pill->setAlignment(Qt::AlignCenter);
	pill->setContentsMargins(Style::getDimension(json::global::unit_0_5), 0,
				 Style::getDimension(json::global::unit_0_5), 0);
	Style::setStyle(pill, style::properties::label::menuSmall);
	Style::setStyle(pill, style::properties::label::statusLabel, variant);
	return pill;
}

MenuSectionCollapseWidget *InstrumentTemplate::createMenuSection(const QString &title, SettingOwner owner,
								 QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		title, MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QLabel *pill = createOwnerPill(owner, section);

	// MenuCollapseHeader's layout is [headerWidget][spacer][collapse ctrl], so index 1
	// puts the pill between the title and the spacer. MHW_COMPOSITEWIDGET would host it
	// too, but CompositeHeaderWidget stacks vertically and would drop it under the title.
	MenuCollapseHeader *header = qobject_cast<MenuCollapseHeader *>(section->collapseSection()->header());
	QHBoxLayout *hlay = header ? qobject_cast<QHBoxLayout *>(header->layout()) : nullptr;

	if(hlay) {
		hlay->insertWidget(1, pill);
		// The header's own spacer is horizontally Minimum, so once the title stops
		// being greedy nothing absorbs the slack and the layout spreads it evenly,
		// centering title and pill. This takes it instead, so both stay left.
		hlay->insertStretch(2, 1);

		// Both the title and its BaseHeaderWidget wrapper grow, and the header's own
		// spacer doesn't, so left alone they take all the slack and shove the pill
		// against the collapse arrow. Make both hug the text.
		//
		// QLineEdit's sizeHint ignores its text, so the width has to be measured —
		// after ensurePolished(), since the bold 14px comes from
		// lineedit::headerLineEdit and unpolished metrics would come out too narrow.
		QWidget *titleWidget = header->headerWidget();
		titleWidget->setSizePolicy(QSizePolicy::Maximum, titleWidget->sizePolicy().verticalPolicy());
		if(QLineEdit *label = titleWidget->findChild<QLineEdit *>()) {
			label->ensurePolished();
			label->setSizePolicy(QSizePolicy::Maximum, label->sizePolicy().verticalPolicy());
			label->setMinimumWidth(0);
			label->setFixedWidth(label->fontMetrics().horizontalAdvance(title) +
					     Style::getDimension(json::global::unit_1));
		}
	} else {
		section->add(pill);
	}

	return section;
}

QPushButton *InstrumentTemplate::debugBtn() const { return m_debugBtn; }

DebugPopup *InstrumentTemplate::debugPopup()
{
	if(!m_debugPopup) {
		m_debugPopup = new DebugPopup(m_tool);
		// Tall enough that a pipeline tree or a DataStore table is worth reading — a
		// panel sized to a QTabWidget's own hint comes out about two rows tall.
		m_debugPopup->setMinimumHeight(debugPanelHeight());
		m_debugPopup->setVisible(false);

		// Docked into ToolTemplate's own column, immediately above the bottom rail:
		// that lands it in the cell the rail spans, so it is exactly as wide as the
		// rail with no geometry computed by hand, and it takes height from the layout
		// instead of covering the plot.
		//
		// Stretch 0 against `content`'s MinimumExpanding: the panel settles at the
		// height it needs and the plot absorbs whatever is left, so resizing the
		// window grows the plot rather than the debug tabs.
		QVBoxLayout *col = qobject_cast<QVBoxLayout *>(m_tool->layout());
		if(col) {
			int railIdx = col->indexOf(m_tool->bottomContainer());
			col->insertWidget(railIdx < 0 ? -1 : railIdx, m_debugPopup, 0);
		}
	}
	return m_debugPopup;
}

void InstrumentTemplate::addDebugTab(QWidget *w, const QString &title) { debugPopup()->addTab(w, title); }

#include "moc_instrumenttemplate.cpp"
