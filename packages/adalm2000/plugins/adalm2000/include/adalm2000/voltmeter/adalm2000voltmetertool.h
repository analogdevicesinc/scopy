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
 */

#ifndef ADALM2000VOLTMETERTOOL_H
#define ADALM2000VOLTMETERTOOL_H

#include "scopy-adalm2000_export.h"
#include "voltmeterscale.h"

#include <QLabel>
#include <QString>
#include <QVector>
#include <QWidget>

#include <qcoro/qcorotask.h>

#include <gui/widgets/toolbuttons.h>
#include <pluginbase/toolmenuentry.h>
#include <tooltemplate.h>
#include <style.h>

class QTimer;
class QVBoxLayout;

namespace scopy {
class FileBrowserWidget;
class MenuCombo;
class MenuOnOffSwitch;
class PlotChannel;
class PlotWidget;
class ValueBarWidget;
class ValueMonitorWidget;

namespace gui {
class MenuPlotAxisRangeControl;
class MenuSpinbox;
class PlotAutoscaler;
} // namespace gui

namespace component {
class Attribute;
class Context;
} // namespace component

namespace adalm2000 {
class Adalm2000Voltmeter_API;
class M2kVoltmeterCsvLogger;
class M2kVoltmeterController;
class M2kVoltmeterReader;

class SCOPY_ADALM2000_EXPORT Adalm2000VoltmeterTool : public QWidget
{
	Q_OBJECT
	friend class Adalm2000Voltmeter_API;

public:
	explicit Adalm2000VoltmeterTool(ToolMenuEntry *tme, component::Context *ctx, const QString &uri,
					QWidget *parent = nullptr);
	~Adalm2000VoltmeterTool() override = default;

private:
	void setupUi();
	component::Attribute *findRangeAttribute(int channel) const;
	QWidget *createChannelSection(int channel, QWidget *parent);
	void applyScalePreset(int channel, int index);
	void createChannelPlot(int channel, QWidget *parent);
	QWidget *createPlotSettings(int channel, QWidget *parent);
	void setPlotHistory(int channel, double seconds);
	void clearPlotHistory(int channel);
	void pushPlotSample(int channel, double value);
	void repointPlotCurve(int channel);
	void applyPlotYRange(int channel, double min, double max);
	void setPlotAutoscaleEnabled(int channel, bool on);
	void setPlotVisible(int channel, bool on);

	void onReadings(int channel, double dcVolts, double acVolts);
	void onRangeChangeRequested(int channel, bool lowRange);
	void onModeChanged(int channel, int index);
	void onGainSelected(int channel, int index);
	void writeRange(int channel, bool lowRange);
	void resetPeakHold();
	QCoro::Task<void> startRunning();

	QColor railColor(int channel) { return Style::getColor(channel == 0 ? json::global::ch0 : json::global::ch1); }

	ToolMenuEntry *m_tme;
	component::Context *m_ctx;
	QString m_uri;

	ToolTemplate *m_tool = nullptr;
	GearBtn *m_gearBtn = nullptr;
	RunBtn *m_runBtn = nullptr;

	ValueMonitorWidget *m_monitor[2] = {nullptr, nullptr};
	ValueBarWidget *m_scale[2] = {nullptr, nullptr};
	RangePresetScaler m_autoScaler[2];
	QTimer *m_scaleTimer = nullptr;
	MenuCombo *m_modeCombo[2] = {nullptr, nullptr};
	MenuCombo *m_gainCombo[2] = {nullptr, nullptr};
	QLabel *m_activeRangeLabel[2] = {nullptr, nullptr};
	MenuOnOffSwitch *m_peakHoldSwitch = nullptr;

	QVBoxLayout *m_centralLayout = nullptr;
	PlotWidget *m_plot[2] = {nullptr, nullptr};
	PlotChannel *m_plotCh[2] = {nullptr, nullptr};
	gui::PlotAutoscaler *m_plotAutoscaler[2] = {nullptr, nullptr};
	gui::MenuPlotAxisRangeControl *m_plotYRange[2] = {nullptr, nullptr};
	gui::MenuSpinbox *m_historySpin[2] = {nullptr, nullptr};
	MenuOnOffSwitch *m_plotVisibleSwitch[2] = {nullptr, nullptr};
	MenuOnOffSwitch *m_autoscaleSwitch[2] = {nullptr, nullptr};

	QVector<double> m_histX[2];
	QVector<double> m_histY[2];
	int m_histCap[2] = {0, 0};
	double m_pendingYMin[2] = {0.0, 0.0};

	bool m_acMode[2] = {false, false};

	M2kVoltmeterController *m_controller = nullptr;
	M2kVoltmeterReader *m_reader = nullptr;

	double m_peakMin[2] = {0.0, 0.0};
	double m_peakMax[2] = {0.0, 0.0};
	bool m_peakSeen[2] = {false, false};

	int m_lastRangeLow[2] = {-1, -1};

	double m_lastReading[2] = {0.0, 0.0};

	M2kVoltmeterCsvLogger *m_logger = nullptr;
	MenuOnOffSwitch *m_logSwitch = nullptr;
	scopy::FileBrowserWidget *m_logFilePath = nullptr;

	Adalm2000Voltmeter_API *m_api = nullptr;

	const QString m_settingsMenuId = "settings";
};

} // namespace adalm2000
} // namespace scopy
#endif // ADALM2000VOLTMETERTOOL_H
