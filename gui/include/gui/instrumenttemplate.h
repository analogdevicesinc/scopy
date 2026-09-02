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

#ifndef INSTRUMENTTEMPLATE_H
#define INSTRUMENTTEMPLATE_H

#include "scopy-gui_export.h"
#include "tooltemplate.h"
#include "widgets/menucontrolbutton.h"
#include "widgets/menusectionwidget.h"
#include "widgets/verticalchannelmanager.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QSplitter>
#include <QTabWidget>
#include <QWidget>

namespace scopy {

class RunBtn;
class SingleShotBtn;
class InfoBtn;
class PrintIconBtn;

/**
 * Which object owns the setting a menu section writes. Rendered as a small pill in the
 * section header — see docs/acq-engine/09-settings-scope.md.
 */
enum SettingOwner
{
	SO_VIEW, // local view state, instant, never reaches the engine
	SO_CH,	 // writes that channel's block
	SO_DEV,	 // writes the one device-wide block
};

/**
 * The four layouts wrapping the center widget. Anything that *annotates* plots —
 * measurements, stats, cursor readouts, marker lists — goes into one of these rather than
 * getting its own hardcoded region. See docs/acq-engine/08-instrument-layout.md.
 */
enum PlotSlot
{
	PS_TOP,
	PS_LEFT,
	PS_RIGHT,
	PS_BOTTOM,
};

/**
 * The debug panel raised from the bottom rail. A tabbed host, docked under the rail and
 * hidden when closed, so it costs no plot height until it is opened.
 */
class SCOPY_GUI_EXPORT DebugPopup : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	DebugPopup(QWidget *parent = nullptr);
	~DebugPopup();

	// Adds a tab. Takes ownership through the QTabWidget.
	void addTab(QWidget *w, const QString &title);
	QTabWidget *tabs() const;

private:
	QTabWidget *m_tabs;
};

/**
 * A reusable instrument shell. Fixes *where things live* per
 * docs/acq-engine/08-instrument-layout.md, and nothing about what they look like:
 *
 *   +--------------------------------------------------------------------+
 *   | top rail  info print export | engine ctrls | fps/target | Run Single |  transparent
 *   +----------+----------------------------------------+--------------+
 *   | channel  |            PS_TOP slot                 |  right menu  |  rails/center are
 *   | rail     |  PS_LEFT | center widget  | PS_RIGHT   |  (one page   |  transparent, only
 *   | (groups  |          | (a PlotManager,|            |   per thing) |  the two menus get
 *   |  of rows)|          |  nothing else) |            |              |  background_primary
 *   |          |            PS_BOTTOM slot              |              |
 *   +----------+----------------------------------------+--------------+
 *   | bottom rail   Debug                                               |  transparent
 *   +--------------------------------------------------------------------+
 *
 * The shell knows nothing about acquisition: no block, plot or channel type is named
 * here. Owners hand it widgets and it places them.
 */
class SCOPY_GUI_EXPORT InstrumentTemplate : public QWidget
{
	Q_OBJECT
public:
	InstrumentTemplate(QWidget *parent = nullptr);
	~InstrumentTemplate();

	// ---- regions -------------------------------------------------------------

	ToolTemplate *tool() const;

	// The [channel rail | center | right menu] splitter. Exposed so an owner can read or
	// restore the widths it was dragged to; the shell needs nothing from it.
	QSplitter *splitter() const;

	// The truly central widget — a PlotManager and nothing else. Everything that
	// annotates it goes into a slot.
	void setCenterWidget(QWidget *w);
	QWidget *centerWidget() const;

	// The four slot layouts around the center. Empty slots collapse to zero, so an
	// instrument that uses none looks like a plain plot stack.
	//
	//   m_it->addToSlot(PS_BOTTOM, measurementsPanel);   // that's the whole API
	void addToSlot(PlotSlot slot, QWidget *w);
	void removeFromSlot(PlotSlot slot, QWidget *w);
	QBoxLayout *slot(PlotSlot slot) const;

	void addToTopRail(QWidget *w, ToolTemplateAlignment a = TTA_LEFT);
	void addToBottomRail(QWidget *w, ToolTemplateAlignment a = TTA_LEFT);

	// ---- top rail ------------------------------------------------------------
	// Built up front, in doc order: info · print · export | <engine controls> |
	// fps readout over target-fps field, with Run/Single pinned right. Owners wire
	// the signals and fill the gap in the middle with addEngineControl().

	RunBtn *runBtn() const;
	SingleShotBtn *singleBtn() const;
	InfoBtn *infoBtn() const;
	PrintIconBtn *printBtn() const;
	QPushButton *exportBtn() const;

	// Inserts an engine control (a MenuCombo, MenuSpinbox, …) after the icon buttons
	// and before the fps readout, preserving call order. The controls and the fps
	// readout share one filled section — the only background in the top rail.
	//
	// The control is forced to a preferred width: MenuCombo and MenuSpinbox both
	// default to Expanding, and in a rail that means one control swallows the row.
	void addEngineControl(QWidget *w);

	// The filled section holding the engine controls and the fps readout.
	QWidget *engineSection() const;

	// Plot-agnostic fps readout: same 10-sample rolling average as FPSInfo, but driven
	// by the owner instead of by a PlotWidget. Call once per frame.
	void setFps(qint64 timestampMs);
	QLabel *fpsLabel() const;

	// The editable target under the readout. The shell only collects the number and
	// emits targetFpsChanged — what a frame rate means is the owner's business, so
	// nothing here touches an engine.
	//
	// 0 is "no ceiling", which is both what an empty field means and what
	// AcquisitionEngine::setMaxFPS(0) means, so the two agree without translation.
	int targetFps() const;
	void setTargetFps(int fps);
	QLineEdit *targetFpsEdit() const;

	// ---- channel rail --------------------------------------------------------

	VerticalChannelManager *channelRail() const;

	// A named, collapsible group of rows — Sources · Processors · Decoders · Plots.
	// Repeat calls with the same title return the existing group.
	MenuSectionCollapseWidget *addChannelGroup(const QString &title);

	// One row: select · colour · name · gear. The row joins the rail's exclusive
	// selection group; clicking it (or its gear) shows the menu page registered
	// under `menuId`. Pass an empty menuId for a row with no page of its own.
	// `indent` shifts the row right, which is how processor stacking is shown.
	//
	// `group` is any rail container — a group from addChannelGroup() or an expandable
	// row from addExpandableChannelRow(), which is how a source's channels are nested
	// under it. An invalid `color` leaves the row's own default, which is what
	// everything that is not a plot channel wants: colour on the rail means "this is
	// the curve you see in that colour", and a source or processor has no curve.
	MenuControlButton *addChannelRow(CompositeWidget *group, const QString &name, const QColor &color,
					 const QString &menuId, int indent = 0);

	// Same row, but with an on/off switch instead of a radio — plots create and
	// destroy rather than mute, so they get a switch.
	MenuControlButton *addChannelSwitchRow(CompositeWidget *group, const QString &name, const QColor &color,
					       const QString &menuId, int indent = 0);

	// A row that is itself a container: the row on top, its children indented under
	// it, and an arrow on the row that collapses them. This is how a source carries
	// its channels — pass the returned widget back as the `group` of the child rows.
	//
	// The row behaves like addChannelRow's otherwise: it joins the exclusive selection
	// group and opens `menuId`. Its arrow is a separate hit target from the row body,
	// so expanding does not change which page is showing, and vice versa.
	CollapsableMenuControlButton *addExpandableChannelRow(CompositeWidget *group, const QString &name,
							      const QColor &color, const QString &menuId);

	// The switch inside a row built by addChannelSwitchRow, so an owner can bind it
	// without knowing how the row is assembled. Null for a row built by
	// addChannelRow — those have a checkbox instead.
	static SmallOnOffSwitch *rowSwitch(MenuControlButton *row);

	// Undoes addChannelRow/addChannelSwitchRow. Four steps rather than one because
	// the row is registered in three places: the exclusive button group (which would
	// otherwise keep a dangling pointer), the group's layout, and the menu stack.
	// `menuId` may be empty for a row that registered no page. `deletePage` forwards
	// to removeMenuPage(): pass true unless you keep the page yourself.
	void removeChannelRow(CompositeWidget *group, MenuControlButton *row, const QString &menuId,
			      bool deletePage = false);

	// A row pinned above the scroll area, outside every group. Used for the
	// device/general-settings entry.
	MenuControlButton *addRailHeaderRow(const QString &name, const QString &menuId);

	QButtonGroup *channelGroup() const;

	// ---- right menu ----------------------------------------------------------

	// Hosts a page. Built once, never rebuilt; showMenuPage() only switches.
	void addMenuPage(const QString &id, QWidget *w);
	// Unstacks the page. Defaults to leaving it alive — several existing callers keep
	// their own pointer and re-add it later — so pass `deletePage` to be rid of it
	// rather than leaving an orphaned hidden top-level behind.
	void removeMenuPage(const QString &id, bool deletePage = false);
	bool hasMenuPage(const QString &id) const;
	void showMenuPage(const QString &id);

	// A collapsible section carrying an owner pill in its header. This is the unit
	// a menu page is assembled from.
	MenuSectionCollapseWidget *createMenuSection(const QString &title, SettingOwner owner,
						     QWidget *parent = nullptr);

	// The pill on its own, for callers assembling a header by hand.
	static QLabel *createOwnerPill(SettingOwner owner, QWidget *parent = nullptr);

	// ---- bottom rail ---------------------------------------------------------

	// The Debug toggle and its popup. The popup is created on first use.
	QPushButton *debugBtn() const;
	DebugPopup *debugPopup();
	void addDebugTab(QWidget *w, const QString &title);

Q_SIGNALS:
	// Emitted when a rail row is selected, with the menu id it registered.
	void menuPageSelected(QString id);

	// The reader edited the target fps field. 0 means they cleared it — no ceiling.
	// Only on editingFinished, so a rate is never applied half-typed.
	void targetFpsChanged(int fps);

private:
	void setupRegions();
	void setupSlots();
	void setupTopRail();
	void setupBottomRail();
	// Shared by all three add*Row overloads. `row` is the button that goes in the rail
	// for a plain row, or the header of the expandable one — everything except which
	// widget gets inserted into `group` is identical between them.
	void configureChannelRow(MenuControlButton *row, QWidget *insert, CompositeWidget *group, const QString &name,
				 const QColor &color, const QString &menuId, int indent);
	MenuControlButton *makeChannelRow(CompositeWidget *group, const QString &name, const QColor &color,
					  const QString &menuId, int indent, bool asSwitch);

	ToolTemplate *m_tool;

	// the three draggable regions, replacing ToolTemplate's fixed-width side containers
	QSplitter *m_splitter;
	QWidget *m_leftPanel;
	QWidget *m_rightPanel;

	// center: the slot frame, and the one widget at its heart
	QWidget *m_centerFrame;
	QWidget *m_center;
	QHBoxLayout *m_centerRow;
	QVBoxLayout *m_centerCol;
	QMap<int, QBoxLayout *> m_slots;

	// top rail
	InfoBtn *m_infoBtn;
	PrintIconBtn *m_printBtn;
	QPushButton *m_exportBtn;
	// Stacked: measured rate above, the target the reader asks for below.
	QLabel *m_fps;
	QLineEdit *m_targetFps;
	QWidget *m_engineSection;
	QWidget *m_engineControls;
	QHBoxLayout *m_engineControlsLay;
	RunBtn *m_runBtn;
	SingleShotBtn *m_singleBtn;

	// fps rolling average, mirroring FPSInfo
	QList<qint64> m_frameTimes;
	qint64 m_lastFrameStamp;

	// channel rail
	VerticalChannelManager *m_vcm;
	QButtonGroup *m_channelGroup;
	QMap<QString, MenuSectionCollapseWidget *> m_groups;

	// bottom rail
	QPushButton *m_debugBtn;
	DebugPopup *m_debugPopup;
};
} // namespace scopy

#endif // INSTRUMENTTEMPLATE_H
