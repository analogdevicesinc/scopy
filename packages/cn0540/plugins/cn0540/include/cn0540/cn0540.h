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

#ifndef CN0540_H
#define CN0540_H

#include "scopy-cn0540_export.h"

#include <optional>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <qcorotask.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/widgets/menusectionwidget.h>

namespace scopy {
namespace component {
class Context;
class Device;
class Channel;
} // namespace component

namespace cn0540 {

static constexpr int NUM_ANALOG_PINS = 6;

class CN0540_API;

class SCOPY_CN0540_EXPORT CN0540 : public QWidget
{
	Q_OBJECT

	friend class CN0540_API;

public:
	explicit CN0540(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
	~CN0540();

Q_SIGNALS:
	void readAll();

private Q_SLOTS:
	QCoro::Task<void> onReadSwFF();
	QCoro::Task<void> onReadVshift();
	QCoro::Task<void> onReadVsensor();
	QCoro::Task<void> onCalibrate();
	QCoro::Task<void> updateVoltages();

private:
	void setupUi();
	void findGpioChannels();
	void findVoltMonChannels();
	QCoro::Task<bool> getGpioState(component::Channel *ch);
	QCoro::Task<void> setGpioState(component::Channel *ch, bool state);
	QCoro::Task<double> getVoltage(component::Channel *ch);
	QCoro::Task<void> setVoltage(component::Channel *ch, double voltageMv);
	QCoro::Task<double> getVshiftMv();

	MenuSectionCollapseWidget *createPowerControlSection(QWidget *parent);
	MenuSectionCollapseWidget *createAdcDriverSection(QWidget *parent);
	MenuSectionCollapseWidget *createSensorCalibSection(QWidget *parent);
	MenuSectionCollapseWidget *createVoltMonSection(QWidget *parent);

	component::Context *m_ctx;
	component::Device *m_adcDev;
	component::Device *m_dacDev;
	component::Device *m_gpioDev;
	component::Device *m_voltMonDev;

	component::Channel *m_adcCh;
	component::Channel *m_dacCh;

	component::Channel *m_gpioSwFF;
	component::Channel *m_gpioShutdown;
	component::Channel *m_gpioFdaDis;
	component::Channel *m_gpioFdaMode;
	component::Channel *m_gpioCC;

	component::Channel *m_analogIn[NUM_ANALOG_PINS];

	IIOWidgetGroup *m_group;

	std::optional<QCoro::Task<void>> m_calibTask;
	bool m_calibInFlight = false;
	bool m_voltMonInFlight = false;
	QTimer *m_voltMonTimer;

	QLabel *m_swffStatusLabel;
	QLabel *m_sensorVoltageLabel;
	QLabel *m_calibStatusLabel;
	QLabel *m_voltMonLabels[NUM_ANALOG_PINS];

	QLineEdit *m_vshiftLineEdit;

	IIOWidget *m_shutdownWidget;
	IIOWidget *m_ccWidget;
	IIOWidget *m_fdaWidget;
	IIOWidget *m_fdaModeWidget;

	AnimatedRefreshBtn *m_refreshBtn;

	ToolTemplate *m_tool;

	bool m_isXadc = false;

	// Physical constants — shared with CN0540_API via friend class
	static constexpr double DAC_BUF_GAIN = 1.22;
};

} // namespace cn0540
} // namespace scopy
#endif // CN0540_H
