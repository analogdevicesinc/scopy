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

#ifndef ADALM2000POWERSUPPLYTOOL_H
#define ADALM2000POWERSUPPLYTOOL_H

#include "powersupplymath.h"
#include "scopy-adalm2000_export.h"

#include <QLabel>
#include <QString>
#include <QWidget>

#include <gui/widgets/toolbuttons.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <tooltemplate.h>

namespace scopy {
class ValueBarWidget;
}
class QPushButton;
class QSlider;

namespace scopy {
class CustomSwitch;
class ValueMonitorWidget;

namespace component {
class Context;
}

namespace adalm2000 {
class M2kPowerSupplyController;

class SCOPY_ADALM2000_EXPORT Adalm2000PowerSupplyTool : public QWidget
{
	Q_OBJECT

public:
	explicit Adalm2000PowerSupplyTool(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
	~Adalm2000PowerSupplyTool() override;

protected:
	void showEvent(QShowEvent *event) override;
	void hideEvent(QHideEvent *event) override;

private:
	void setupUi();
	QWidget *createRailReadout(int channel, QWidget *parent);
	void createRailSettings(int channel, QWidget *parent);
	void applyVoltageScale(IIOWidget *widget, int channel);

	void onMeasured(int channel, double volts);
	void onSetpointChanged(int channel, double volts);
	void onEnableToggled(int channel, bool on);
	void onTrackingModeChanged(bool tracking);
	void onRatioChanged(int percent);
	void applyTracking();

	component::Context *m_ctx;
	IIOWidgetGroup *m_group;
	M2kPowerSupplyController *m_controller = nullptr;

	ToolTemplate *m_tool = nullptr;
	GearBtn *m_gearBtn = nullptr;

	ValueMonitorWidget *m_setMonitor[2] = {nullptr, nullptr};
	ValueMonitorWidget *m_measuredMonitor[2] = {nullptr, nullptr};
	ValueBarWidget *m_scale[2] = {nullptr, nullptr};
	QPushButton *m_enableBtn[2] = {nullptr, nullptr};
	IIOWidget *m_voltageWidget[2] = {nullptr, nullptr};

	// Checked lights the LEFT label, "Independent", so checked == NOT tracking.
	CustomSwitch *m_modeSwitch = nullptr;
	QSlider *m_ratioSlider = nullptr;
	QLabel *m_ratioLabel = nullptr;

	ps::RailAverage m_average[2];
	double m_setpoint[2] = {0.0, 0.0};
	bool m_railOn[2] = {false, false};
	bool m_tracking = false;

	const QString m_settingsMenuId = "settings";
};

} // namespace adalm2000
} // namespace scopy
#endif // ADALM2000POWERSUPPLYTOOL_H
