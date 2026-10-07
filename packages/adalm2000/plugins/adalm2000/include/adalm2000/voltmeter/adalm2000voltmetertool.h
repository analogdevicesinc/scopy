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
#include <QWidget>

#include <qcoro/qcorotask.h>

#include <gui/widgets/toolbuttons.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <pluginbase/toolmenuentry.h>
#include <tooltemplate.h>
#include <style.h>

class QTimer;

namespace scopy {
class MenuCombo;
class MenuOnOffSwitch;
class ValueBarWidget;
class ValueMonitorWidget;

namespace component {
class Attribute;
class Context;
} // namespace component

namespace adalm2000 {
class M2kVoltmeterController;
class M2kVoltmeterReader;

class SCOPY_ADALM2000_EXPORT Adalm2000VoltmeterTool : public QWidget
{
	Q_OBJECT

public:
	explicit Adalm2000VoltmeterTool(ToolMenuEntry *tme, component::Context *ctx, IIOWidgetGroup *group,
					const QString &uri, QWidget *parent = nullptr);
	~Adalm2000VoltmeterTool() override = default;

private:
	void setupUi();
	component::Attribute *findRangeAttribute(int channel) const;
	QWidget *createChannelSection(int channel, QWidget *parent);
	void applyScalePreset(int channel, int index);

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
	IIOWidgetGroup *m_group;
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

	bool m_acMode[2] = {false, false};

	M2kVoltmeterController *m_controller = nullptr;
	M2kVoltmeterReader *m_reader = nullptr;

	double m_peakMin[2] = {0.0, 0.0};
	double m_peakMax[2] = {0.0, 0.0};
	bool m_peakSeen[2] = {false, false};

	int m_lastRangeLow[2] = {-1, -1};

	const QString m_settingsMenuId = "settings";
};

} // namespace adalm2000
} // namespace scopy
#endif // ADALM2000VOLTMETERTOOL_H
