/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include <QFuture>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <iio.h>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/widgets/menusectionwidget.h>

namespace scopy::cn0540 {

static constexpr int NUM_ANALOG_PINS = 6;

class CN0540_API;

class SCOPY_CN0540_EXPORT CN0540 : public QWidget
{
	Q_OBJECT

	friend class CN0540_API;

public:
	explicit CN0540(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
	~CN0540();

Q_SIGNALS:
	void readAll();

private Q_SLOTS:
	void onReadSwFF();
	void onReadVshift();
	void onReadVsensor();
	void onCalibrate();
	void updateVoltages();

private:
	void setupUi();
	void findGpioChannels();
	void findVoltMonChannels();
	bool getGpioState(iio_channel *ch);
	void setGpioState(iio_channel *ch, bool state);
	double getVoltage(iio_channel *ch);
	void setVoltage(iio_channel *ch, double voltageMv);
	double getVshiftMv();

	MenuSectionCollapseWidget *createPowerControlSection(QWidget *parent);
	MenuSectionCollapseWidget *createAdcDriverSection(QWidget *parent);
	MenuSectionCollapseWidget *createSensorCalibSection(QWidget *parent);
	MenuSectionCollapseWidget *createVoltMonSection(QWidget *parent);

	iio_context *m_ctx;
	iio_device *m_adcDev;
	iio_device *m_dacDev;
	iio_device *m_gpioDev;
	iio_device *m_voltMonDev;

	iio_channel *m_adcCh;
	iio_channel *m_dacCh;

	iio_channel *m_gpioSwFF;
	iio_channel *m_gpioShutdown;
	iio_channel *m_gpioFdaDis;
	iio_channel *m_gpioFdaMode;
	iio_channel *m_gpioCC;

	iio_channel *m_analogIn[NUM_ANALOG_PINS];

	IIOWidgetGroup *m_group;

	QFuture<void> m_calibFuture;
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

} // namespace scopy::cn0540
#endif // CN0540_H
