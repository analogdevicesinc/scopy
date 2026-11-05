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

#ifndef MULTIACQ_MULTIACQTOOL_H
#define MULTIACQ_MULTIACQTOOL_H

#include <core/acq_engine/datakey.h>

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QWidget>

class QTimer;

namespace scopy {
class CollapsableMenuControlButton;
class DockableAreaInterface;
class DockWrapperInterface;
class MenuCombo;
class MenuControlButton;
class MenuSectionCollapseWidget;
class ToolMenuEntry;
namespace component {
class Context;
}
namespace adc {
class AcqInstrument;
}

namespace multiacq {

class IioSource;
class TimeCurve;
class TimePlot;

// The tool: an acquisition engine with no device of its own. The reader adds devices by
// URI, each one an IioSource, then adds time plots and puts any device's channels on any
// plot. Nothing is created until asked for.
class MultiAcqTool : public QWidget
{
	Q_OBJECT
public:
	explicit MultiAcqTool(ToolMenuEntry *tme, QWidget *parent = nullptr);
	~MultiAcqTool() override;

private:
	struct DeviceEntry
	{
		IioSource *src{nullptr};
		MenuControlButton *row{nullptr};
		QString menuId;
	};

	struct PlotEntry
	{
		TimePlot *plot{nullptr};
		DockWrapperInterface *dock{nullptr};
		CollapsableMenuControlButton *row{nullptr};
		MenuCombo *picker{nullptr};
		MenuSectionCollapseWidget *channelList{nullptr};
		// Each curve's rail row and its line in the plot page.
		QMap<TimeCurve *, QPair<MenuControlButton *, QWidget *>> curves;
	};

	void setupTopRail();
	void setupDevicesRail();
	void setupPlotsRail();
	void setupFrameTimer();

	// Empty on success, else what went wrong.
	QString addDevice(const QString &uri);
	void removeDevice(IioSource *src);
	QWidget *createDevicePage(IioSource *src);

	TimePlot *addPlot(const QString &name);
	void removePlot(TimePlot *plot);
	QWidget *createPlotPage(PlotEntry *entry);
	PlotEntry *plotEntry(TimePlot *plot) const;
	void addChannel(TimePlot *plot, const scopy::acq::DataKey &yKey);
	void removeChannel(TimePlot *plot, TimeCurve *curve);
	QColor nextColor(TimePlot *plot) const;
	void refreshPickers();
	void fillPicker(MenuCombo *picker) const;

	IioSource *sourceById(const QString &id) const;
	QString channelLabel(const scopy::acq::DataKey &yKey) const;
	QString nextSourceId() const;
	QString deviceName(scopy::component::Context *ctx, const QString &uri) const;
	// Restarts the engine, for changes that only apply at start.
	void scheduleRestart();
	void clearData();
	void setStartTime(double ms);
	void updatePlots();

	QPointer<ToolMenuEntry> m_tme;
	scopy::adc::AcqInstrument *m_ui{nullptr};
	DockableAreaInterface *m_dockArea{nullptr};
	MenuSectionCollapseWidget *m_devicesGroup{nullptr};
	MenuSectionCollapseWidget *m_plotsGroup{nullptr};
	QList<DeviceEntry> m_devices;
	QList<PlotEntry *> m_plots; // owned
	QTimer *m_frameTimer{nullptr};
	QTimer *m_restartTimer{nullptr};
	// Relative time labels count from here; set again by the first run after a clear.
	bool m_needStartTime{true};
	double m_startTime{0.0};
	quint32 m_plotUuid{0};
};

} // namespace multiacq
} // namespace scopy

#endif // MULTIACQ_MULTIACQTOOL_H
