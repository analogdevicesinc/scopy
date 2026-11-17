/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef MONITORVIEW_H
#define MONITORVIEW_H

#include "attributemonitor.h"
#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QButtonGroup;
class QLabel;
class QPushButton;
class QShowEvent;
class QSplitter;
class QTableWidget;
class QTimer;
class QVBoxLayout;

namespace scopy {
class MenuCombo;
class MenuControlButton;
class MenuOnOffSwitch;
class PlotChannel;
class PlotWidget;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class Attribute;
class Channel;
} // namespace scopy::component

namespace scopy::dcdebugger {
class SideSplitter;

// Monitor tab of a Context or Device: polls one attribute per channel (like Data Monitor) and
// plots its history. Scopy instrument layout: channel list on the left grouped by device (a
// channel's checkbox enables it), the channel's menu (source attribute, plot) or the Settings
// menu (Polling and Plots sections) on the right, stacked plots with a value / min / max / mean
// table below. Starts with one plot; the Plots section adds and removes plots.
class SCOPY_DCDEBUGGER_EXPORT MonitorView : public QWidget
{
	Q_OBJECT
public:
	explicit MonitorView(QObject *root, QWidget *parent = nullptr);
	~MonitorView() override;

	AttributeMonitor *monitor() const { return m_monitor; }

	// Channels of `root` (a Context's devices, or a Device) that have attributes to poll.
	static QList<component::Channel *> monitorableChannels(QObject *root);
	static constexpr int MaxPlots = 8;
	// Initial height of the measurements table, at most a quarter of the view.
	static constexpr int MeasurementsHeight = 140;

	int plotCount() const { return int(m_plots.size()); }
	PlotWidget *plot(int index) const;
	// Plot a channel (by monitor key) is drawn on; -1 for an unknown key.
	int plotIndexOf(const QString &key) const;

public Q_SLOTS:
	// Appends a plot under the others; returns its index, or -1 at MaxPlots.
	int addPlot();
	// Removes a plot; its channels move to the first plot and later plots are renumbered.
	// False for the last plot or an index out of range.
	bool removePlot(int index);

Q_SIGNALS:
	void plotsChanged(int count);

protected:
	void showEvent(QShowEvent *event) override;

private:
	struct Entry
	{
		QString key;
		QPointer<component::Channel> channel;
		QString name;
		QColor color;
		QList<component::Attribute *> attributes;
		MenuControlButton *button = nullptr;
		MenuCombo *source = nullptr;
		MenuCombo *plot = nullptr;
		PlotChannel *curve = nullptr;
		int plotIndex = 0;
	};

	QWidget *buildSettings(QWidget *parent);
	void buildChannels();
	QWidget *buildChannelMenu(Entry &e);
	QWidget *buildPlotSettings(QWidget *parent);
	void buildPlots();
	PlotWidget *createPlot();
	void rebuildPlotsLayout();
	void rebuildPlotCombos();
	void updatePlotControls();
	void applyInitialSizes();
	void applySource(Entry &e);
	void assignPlot(Entry &e, int plotIndex);
	void render();
	void updateTable();
	void updateStatus();
	Entry *entry(const QString &key);

	QPointer<QObject> m_root;
	AttributeMonitor *m_monitor;
	QList<Entry> m_entries;

	ToolTemplate *m_tool;
	SideSplitter *m_sides;
	QPushButton *m_runBtn;
	QPushButton *m_clearBtn;
	QLabel *m_status;
	gui::MenuSpinbox *m_period;
	gui::MenuSpinbox *m_history;
	MenuOnOffSwitch *m_scaled;
	QButtonGroup *m_menuGroup;
	QList<PlotWidget *> m_plots;
	QSplitter *m_splitter = nullptr;
	QWidget *m_plotsArea = nullptr;
	QVBoxLayout *m_plotsLay = nullptr;
	MenuCombo *m_plotSelect = nullptr; // the Plots section's plot list
	QPushButton *m_addPlotBtn = nullptr;
	QPushButton *m_removePlotBtn = nullptr;
	int m_selectedPlot = 0; // selection of m_plotSelect, kept across its rebuilds
	bool m_sized = false;
	QTableWidget *m_table = nullptr;
	QTimer *m_redraw;
	bool m_dirty = false;
};

} // namespace scopy::dcdebugger

#endif // MONITORVIEW_H
