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
#include <QStringList>
#include <QWidget>

class QSettings;
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
//
// The public calls below are what the reader's controls do, for the scripting API and for
// setup files; the controls follow whatever they change.
class MultiAcqTool : public QWidget
{
	Q_OBJECT
public:
	explicit MultiAcqTool(ToolMenuEntry *tme, QWidget *parent = nullptr);
	~MultiAcqTool() override;

	// Devices, one per URI.
	QList<IioSource *> sources() const;
	IioSource *sourceByUri(const QString &uri) const;
	// Connects the URI (a bare address is taken as "ip:"). Empty on success, else what went wrong.
	QString addDevice(const QString &uri);
	void removeDevice(IioSource *src);

	QList<TimePlot *> plots() const;
	TimePlot *plotByUuid(quint32 uuid) const;
	// An empty name gets "Plot N".
	TimePlot *addPlot(const QString &name);
	void removePlot(TimePlot *plot);
	// Puts a device channel on a plot; an invalid colour takes the one least used there. The
	// existing curve if it is there already, null if the key names no channel.
	TimeCurve *addChannel(TimePlot *plot, const scopy::acq::DataKey &yKey, const QColor &color = QColor());
	void removeChannel(TimePlot *plot, TimeCurve *curve);
	TimeCurve *curve(TimePlot *plot, const scopy::acq::DataKey &yKey) const;
	// Through the curve's rail switch, so the switch shows it.
	void setCurveVisible(TimePlot *plot, TimeCurve *curve, bool visible);

	void run();
	void stop();
	void single();
	bool isRunning() const;
	void clearData();

	// The tool's part of a setup file. Each saved device goes to the added one with its URI and
	// board, else to its URI connected now if the same board answers there, else to any added
	// one of the same board, and gets its saved config; the saved plots replace the current
	// ones. Saved devices that match nothing are skipped, with their curves.
	void saveSettings(QSettings &s) const;
	void loadSettings(QSettings &s);
	// What the last load could not restore; empty when it restored everything.
	QStringList loadReport() const { return m_loadReport; }

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

	QWidget *createDevicePage(IioSource *src);

	QWidget *createPlotPage(PlotEntry *entry);
	PlotEntry *plotEntry(TimePlot *plot) const;
	QColor nextColor(TimePlot *plot) const;
	void refreshPickers();
	void fillPicker(MenuCombo *picker) const;

	IioSource *sourceById(const QString &id) const;
	QString channelLabel(const scopy::acq::DataKey &yKey) const;
	QString nextSourceId() const;
	QString deviceName(scopy::component::Context *ctx, const QString &uri) const;
	// Whether `src` is the board a setup saved as `model` with these IIO devices.
	static bool sameBoard(IioSource *src, const QString &model, const QStringList &iioDevices);
	static QStringList iioDeviceNames(IioSource *src);
	// Restarts the engine, for changes that only apply at start.
	void scheduleRestart();
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
	QStringList m_loadReport;
};

} // namespace multiacq
} // namespace scopy

#endif // MULTIACQ_MULTIACQTOOL_H
