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

#include "cn0540.h"

#include <cstring>

#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QScrollArea>
#include <QSpacerItem>
#include <QThread>
#include <QtConcurrent>

#include <gui/style_properties.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_CN0540, "CN0540")

#define G 0.3
#define FDA_VOCM_MV 2500.0
#define FDA_GAIN 2.667
#define DAC_BUF_GAIN 1.22
#define CALIB_MAX_ITER 20
#define XADC_VREF 3.3

using namespace scopy;
using namespace scopy::cn0540;

CN0540::CN0540(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_adcDev(nullptr)
	, m_dacDev(nullptr)
	, m_gpioDev(nullptr)
	, m_voltMonDev(nullptr)
	, m_adcCh(nullptr)
	, m_dacCh(nullptr)
	, m_gpioSwFF(nullptr)
	, m_gpioShutdown(nullptr)
	, m_gpioFdaDis(nullptr)
	, m_gpioFdaMode(nullptr)
	, m_gpioCC(nullptr)
	, m_voltMonTimer(nullptr)
	, m_swffStatusLabel(nullptr)
	, m_shutdownStatusLabel(nullptr)
	, m_fdaStatusLabel(nullptr)
	, m_fdaModeStatusLabel(nullptr)
	, m_ccStatusLabel(nullptr)
	, m_inputVoltageLabel(nullptr)
	, m_sensorVoltageLabel(nullptr)
	, m_calibStatusLabel(nullptr)
	, m_vshiftEdit(nullptr)
	, m_shutdownChk(nullptr)
	, m_fdaChk(nullptr)
	, m_fdaModeChk(nullptr)
	, m_ccChk(nullptr)
	, m_refreshBtn(nullptr)
	, m_tool(nullptr)
{
	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		m_analogIn[i] = nullptr;
		m_voltMonLabels[i] = nullptr;
	}

	if(!m_ctx) {
		qWarning(CAT_CN0540) << "No context provided";
		return;
	}

	m_adcDev = iio_context_find_device(m_ctx, "ad7768-1");
	m_dacDev = iio_context_find_device(m_ctx, "ltc2606");
	m_gpioDev = iio_context_find_device(m_ctx, "one-bit-adc-dac");
	m_voltMonDev = iio_context_find_device(m_ctx, "xadc");
	if(!m_voltMonDev)
		m_voltMonDev = iio_context_find_device(m_ctx, "ltc2308");

	if(m_adcDev)
		m_adcCh = iio_device_find_channel(m_adcDev, "voltage0", false);
	if(m_dacDev)
		m_dacCh = iio_device_find_channel(m_dacDev, "voltage0", true);

	if(m_gpioDev)
		findGpioChannels();

	if(m_voltMonDev)
		findVoltMonChannels();

	setupUi();

	if(m_voltMonDev) {
		m_voltMonTimer = new QTimer(this);
		connect(m_voltMonTimer, &QTimer::timeout, this, &CN0540::updateVoltages);
		m_voltMonTimer->start(1000);
	}

	Q_EMIT readAll();
}

CN0540::~CN0540()
{
	if(m_voltMonTimer)
		m_voltMonTimer->stop();
}

void CN0540::findGpioChannels()
{
	unsigned int nch = iio_device_get_channels_count(m_gpioDev);
	for(unsigned int i = 0; i < nch; i++) {
		iio_channel *ch = iio_device_get_channel(m_gpioDev, i);
		if(!ch)
			continue;

		char label[64] = {};
		int ret = iio_channel_attr_read(ch, "label", label, sizeof(label));
		if(ret < 0)
			continue;

		if(strstr(label, "cn0540_sw_ff_gpio"))
			m_gpioSwFF = ch;
		else if(strstr(label, "cn0540_shutdown_gpio"))
			m_gpioShutdown = ch;
		else if(strstr(label, "cn0540_FDA_DIS"))
			m_gpioFdaDis = ch;
		else if(strstr(label, "cn0540_FDA_MODE"))
			m_gpioFdaMode = ch;
		else if(strstr(label, "cn0540_blue_led"))
			m_gpioCC = ch;
	}
}

void CN0540::findVoltMonChannels()
{
	// xadc analog pins start at voltage9; ltc2308 starts at voltage0
	const char *devName = iio_device_get_name(m_voltMonDev);
	int startIdx = (devName && strcmp(devName, "xadc") == 0) ? 9 : 0;

	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		char name[16];
		snprintf(name, sizeof(name), "voltage%d", startIdx + i);
		m_analogIn[i] = iio_device_find_channel(m_voltMonDev, name, false);
	}
}

bool CN0540::getGpioState(iio_channel *ch)
{
	long long val = 0;
	iio_channel_attr_read_longlong(ch, "raw", &val);
	return (bool)val;
}

void CN0540::setGpioState(iio_channel *ch, bool state)
{
	iio_channel_attr_write_longlong(ch, "raw", (long long)state);
}

double CN0540::getVoltage(iio_channel *ch)
{
	long long raw = 0;
	double scale = 0.0;
	iio_channel_attr_read_longlong(ch, "raw", &raw);
	iio_channel_attr_read_double(ch, "scale", &scale);
	return (double)raw * scale;
}

void CN0540::setVoltage(iio_channel *ch, double voltageMv)
{
	double scale = 0.0;
	iio_channel_attr_read_double(ch, "scale", &scale);
	if(scale != 0.0)
		iio_channel_attr_write_longlong(ch, "raw", (long long)(voltageMv / scale));
}

double CN0540::getVshiftMv()
{
	return getVoltage(m_dacCh) * DAC_BUF_GAIN;
}

void CN0540::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	mainLayout->addWidget(m_tool);

	m_refreshBtn = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshBtn, TTA_RIGHT);
	connect(m_refreshBtn, &AnimatedRefreshBtn::clicked, this, &CN0540::readAll);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setFrameShape(QFrame::NoFrame);

	QWidget *centralWidget = new QWidget(scrollArea);
	QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
	centralLayout->setSpacing(10);
	centralLayout->setContentsMargins(10, 10, 10, 10);

	centralLayout->addWidget(createPowerControlSection(centralWidget));
	centralLayout->addWidget(createAdcDriverSection(centralWidget));
	centralLayout->addWidget(createSensorCalibSection(centralWidget));

	if(m_voltMonDev)
		centralLayout->addWidget(createVoltMonSection(centralWidget));

	centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(centralWidget);
	m_tool->addWidgetToCentralContainerHelper(scrollArea);

	connect(this, &CN0540::readAll, this, &CN0540::onReadSwFF);
	connect(this, &CN0540::readAll, this, &CN0540::onReadShutdown);
	connect(this, &CN0540::readAll, this, &CN0540::onReadFda);
	connect(this, &CN0540::readAll, this, &CN0540::onReadFdaMode);
	connect(this, &CN0540::readAll, this, &CN0540::onReadCC);
	connect(this, &CN0540::readAll, this, &CN0540::onReadVshift);
	connect(this, &CN0540::readAll, this, &CN0540::onReadVsensor);
}

MenuSectionCollapseWidget *CN0540::createPowerControlSection(QWidget *parent)
{
	auto *section = new MenuSectionCollapseWidget("Power Control", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *container = new QWidget(section);
	Style::setBackgroundColor(container, json::theme::background_primary);
	Style::setStyle(container, style::properties::widget::border_interactive);
	QFormLayout *layout = new QFormLayout(container);
	layout->setSpacing(8);
	layout->setContentsMargins(10, 10, 10, 10);

	// SW_FF row
	m_swffStatusLabel = new QLabel("---", container);
	Style::setStyle(m_swffStatusLabel, style::properties::label::menuSmall);
	QPushButton *swffBtn = new QPushButton("Check Status", container);
	Style::setStyle(swffBtn, style::properties::button::basicButton);
	connect(swffBtn, &QPushButton::clicked, this, &CN0540::onReadSwFF);
	QHBoxLayout *swffRow = new QHBoxLayout();
	swffRow->addWidget(swffBtn);
	swffRow->addWidget(m_swffStatusLabel);
	layout->addRow("SW_FF:", swffRow);

	// SHUTDOWN row
	m_shutdownStatusLabel = new QLabel("---", container);
	Style::setStyle(m_shutdownStatusLabel, style::properties::label::menuSmall);
	m_shutdownChk = new QCheckBox(container);
	Style::setStyle(m_shutdownChk, style::properties::checkbox::squareCB);
	connect(m_shutdownChk, &QCheckBox::toggled, this, &CN0540::onShutdownToggled);
	QHBoxLayout *shutdownRow = new QHBoxLayout();
	shutdownRow->addWidget(m_shutdownChk);
	shutdownRow->addWidget(m_shutdownStatusLabel);
	layout->addRow("Shutdown:", shutdownRow);

	// CONSTANT CURRENT row
	m_ccStatusLabel = new QLabel("---", container);
	Style::setStyle(m_ccStatusLabel, style::properties::label::menuSmall);
	m_ccChk = new QCheckBox(container);
	Style::setStyle(m_ccChk, style::properties::checkbox::squareCB);
	connect(m_ccChk, &QCheckBox::toggled, this, &CN0540::onCCToggled);
	QHBoxLayout *ccRow = new QHBoxLayout();
	ccRow->addWidget(m_ccChk);
	ccRow->addWidget(m_ccStatusLabel);
	layout->addRow("Constant Current:", ccRow);

	section->contentLayout()->addWidget(container);
	return section;
}

MenuSectionCollapseWidget *CN0540::createAdcDriverSection(QWidget *parent)
{
	auto *section = new MenuSectionCollapseWidget("ADC Driver Settings", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *container = new QWidget(section);
	Style::setBackgroundColor(container, json::theme::background_primary);
	Style::setStyle(container, style::properties::widget::border_interactive);
	QFormLayout *layout = new QFormLayout(container);
	layout->setSpacing(8);
	layout->setContentsMargins(10, 10, 10, 10);

	// FDA status row
	m_fdaStatusLabel = new QLabel("---", container);
	Style::setStyle(m_fdaStatusLabel, style::properties::label::menuSmall);
	m_fdaChk = new QCheckBox(container);
	Style::setStyle(m_fdaChk, style::properties::checkbox::squareCB);
	connect(m_fdaChk, &QCheckBox::toggled, this, &CN0540::onFdaToggled);
	QHBoxLayout *fdaRow = new QHBoxLayout();
	fdaRow->addWidget(m_fdaChk);
	fdaRow->addWidget(m_fdaStatusLabel);
	layout->addRow("FDA Status:", fdaRow);

	// FDA mode row
	m_fdaModeStatusLabel = new QLabel("---", container);
	Style::setStyle(m_fdaModeStatusLabel, style::properties::label::menuSmall);
	m_fdaModeChk = new QCheckBox(container);
	Style::setStyle(m_fdaModeChk, style::properties::checkbox::squareCB);
	connect(m_fdaModeChk, &QCheckBox::toggled, this, &CN0540::onFdaModeToggled);
	QHBoxLayout *fdaModeRow = new QHBoxLayout();
	fdaModeRow->addWidget(m_fdaModeChk);
	fdaModeRow->addWidget(m_fdaModeStatusLabel);
	layout->addRow("FDA Mode:", fdaModeRow);

	section->contentLayout()->addWidget(container);
	return section;
}

MenuSectionCollapseWidget *CN0540::createSensorCalibSection(QWidget *parent)
{
	auto *section = new MenuSectionCollapseWidget("Sensor Calibration", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *container = new QWidget(section);
	Style::setBackgroundColor(container, json::theme::background_primary);
	Style::setStyle(container, style::properties::widget::border_interactive);
	QGridLayout *layout = new QGridLayout(container);
	layout->setSpacing(8);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setColumnStretch(1, 1);

	// Row 0: Input voltage (read-only display)
	layout->addWidget(new QLabel("Input Voltage [mV]:", container), 0, 0, Qt::AlignRight);
	m_inputVoltageLabel = new QLabel("---", container);
	Style::setStyle(m_inputVoltageLabel, style::properties::label::menuSmall);
	layout->addWidget(m_inputVoltageLabel, 0, 1);

	// Row 1: Shift voltage (editable + read/write)
	layout->addWidget(new QLabel("Shift Voltage [mV]:", container), 1, 0, Qt::AlignRight);
	m_vshiftEdit = new QLineEdit("0.0", container);
	layout->addWidget(m_vshiftEdit, 1, 1);
	QPushButton *readVshiftBtn = new QPushButton("Read", container);
	Style::setStyle(readVshiftBtn, style::properties::button::basicButton);
	connect(readVshiftBtn, &QPushButton::clicked, this, &CN0540::onReadVshift);
	layout->addWidget(readVshiftBtn, 1, 2);
	QPushButton *writeVshiftBtn = new QPushButton("Write", container);
	Style::setStyle(writeVshiftBtn, style::properties::button::basicButton);
	connect(writeVshiftBtn, &QPushButton::clicked, this, &CN0540::onWriteVshift);
	layout->addWidget(writeVshiftBtn, 1, 3);

	// Row 2: Sensor voltage (read-only + read button)
	layout->addWidget(new QLabel("Sensor Voltage [mV]:", container), 2, 0, Qt::AlignRight);
	m_sensorVoltageLabel = new QLabel("---", container);
	Style::setStyle(m_sensorVoltageLabel, style::properties::label::menuSmall);
	layout->addWidget(m_sensorVoltageLabel, 2, 1);
	QPushButton *readVsensorBtn = new QPushButton("Read", container);
	Style::setStyle(readVsensorBtn, style::properties::button::basicButton);
	connect(readVsensorBtn, &QPushButton::clicked, this, &CN0540::onReadVsensor);
	layout->addWidget(readVsensorBtn, 2, 2);

	// Row 3: Calibrate button + status
	m_calibStatusLabel = new QLabel("", container);
	Style::setStyle(m_calibStatusLabel, style::properties::label::menuSmall);
	layout->addWidget(m_calibStatusLabel, 3, 1);
	QPushButton *calibBtn = new QPushButton("Calibrate", container);
	Style::setStyle(calibBtn, style::properties::button::basicButton);
	connect(calibBtn, &QPushButton::clicked, this, &CN0540::onCalibrate);
	layout->addWidget(calibBtn, 3, 2);

	section->contentLayout()->addWidget(container);
	return section;
}

MenuSectionCollapseWidget *CN0540::createVoltMonSection(QWidget *parent)
{
	auto *section = new MenuSectionCollapseWidget("Voltage Monitor", MenuCollapseSection::MHCW_ARROW,
						      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *container = new QWidget(section);
	Style::setBackgroundColor(container, json::theme::background_primary);
	Style::setStyle(container, style::properties::widget::border_interactive);
	QFormLayout *layout = new QFormLayout(container);
	layout->setSpacing(8);
	layout->setContentsMargins(10, 10, 10, 10);

	static const char *pinNames[NUM_ANALOG_PINS] = {"Vin+", "Vgpio2", "Vgpio3", "Vcom", "Vfda+", "Vfda-"};
	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		m_voltMonLabels[i] = new QLabel("---", container);
		layout->addRow(QString(pinNames[i]) + " [mV]:", m_voltMonLabels[i]);
	}

	section->contentLayout()->addWidget(container);
	return section;
}

void CN0540::onReadSwFF()
{
	if(!m_gpioSwFF)
		return;
	bool state = getGpioState(m_gpioSwFF);
	m_swffStatusLabel->setText(state ? "HIGH" : "LOW");
}

void CN0540::onReadShutdown()
{
	if(!m_gpioShutdown)
		return;
	bool enabled = !getGpioState(m_gpioShutdown); // active-low
	m_shutdownChk->blockSignals(true);
	m_shutdownChk->setChecked(enabled);
	m_shutdownChk->blockSignals(false);
	m_shutdownStatusLabel->setText(enabled ? "ENABLED" : "DISABLED");
}

void CN0540::onReadFda()
{
	if(!m_gpioFdaDis)
		return;
	bool enabled = !getGpioState(m_gpioFdaDis); // active-low
	m_fdaChk->blockSignals(true);
	m_fdaChk->setChecked(enabled);
	m_fdaChk->blockSignals(false);
	m_fdaStatusLabel->setText(enabled ? "ENABLED" : "DISABLED");
}

void CN0540::onReadFdaMode()
{
	if(!m_gpioFdaMode)
		return;
	bool state = getGpioState(m_gpioFdaMode);
	m_fdaModeChk->blockSignals(true);
	m_fdaModeChk->setChecked(state);
	m_fdaModeChk->blockSignals(false);
	m_fdaModeStatusLabel->setText(state ? "FULL POWER" : "LOW POWER");
}

void CN0540::onReadCC()
{
	if(!m_gpioCC)
		return;
	bool state = getGpioState(m_gpioCC);
	m_ccChk->blockSignals(true);
	m_ccChk->setChecked(state);
	m_ccChk->blockSignals(false);
	m_ccStatusLabel->setText(state ? "ENABLED" : "DISABLED");
}

void CN0540::onShutdownToggled(bool checked)
{
	if(!m_gpioShutdown)
		return;
	// Shutdown pin is active-low
	setGpioState(m_gpioShutdown, !checked);
	m_shutdownStatusLabel->setText(checked ? "ENABLED" : "DISABLED");
}

void CN0540::onFdaToggled(bool checked)
{
	if(!m_gpioFdaDis)
		return;
	// FDA_DIS pin is active-low
	setGpioState(m_gpioFdaDis, !checked);
	m_fdaStatusLabel->setText(checked ? "ENABLED" : "DISABLED");
}

void CN0540::onFdaModeToggled(bool checked)
{
	if(!m_gpioFdaMode)
		return;
	setGpioState(m_gpioFdaMode, checked);
	m_fdaModeStatusLabel->setText(checked ? "FULL POWER" : "LOW POWER");
}

void CN0540::onCCToggled(bool checked)
{
	if(!m_gpioCC)
		return;
	setGpioState(m_gpioCC, checked);
	m_ccStatusLabel->setText(checked ? "ENABLED" : "DISABLED");
}

void CN0540::onReadVshift()
{
	if(!m_dacCh)
		return;
	double vshiftMv = getVshiftMv();
	m_vshiftEdit->setText(QString::number(vshiftMv, 'f', 4));
}

void CN0540::onWriteVshift()
{
	if(!m_dacCh)
		return;
	bool ok;
	double vshiftMv = m_vshiftEdit->text().toDouble(&ok);
	if(!ok)
		return;
	setVoltage(m_dacCh, vshiftMv / DAC_BUF_GAIN);
}

void CN0540::onReadVsensor()
{
	if(!m_adcCh || !m_dacCh)
		return;
	double vadcMv = getVoltage(m_adcCh);
	double vshiftMv = getVshiftMv();
	double v1St = FDA_VOCM_MV - vadcMv / FDA_GAIN;
	double vsensorMv = ((G + 1.0) * vshiftMv - v1St) / G;
	vsensorMv -= getVoltage(m_adcCh);
	m_sensorVoltageLabel->setText(QString::number(vsensorMv, 'f', 4));
}

void CN0540::onCalibrate()
{
	if(!m_adcCh || !m_dacCh)
		return;

	if(m_calibStatusLabel)
		m_calibStatusLabel->setText("Calibrating...");

	QtConcurrent::run([this]() {
		double adcVoltageMv = 0.0;
		for(int i = 0; i < CALIB_MAX_ITER; i++) {
			adcVoltageMv = getVoltage(m_adcCh);
			double dacVoltageMv = getVoltage(m_dacCh) - adcVoltageMv;
			setVoltage(m_dacCh, dacVoltageMv);
			QThread::msleep(10);
		}
		QString adcStr = QString::number(adcVoltageMv, 'f', 4);
		QMetaObject::invokeMethod(
			this,
			[this, adcStr]() {
				if(m_calibStatusLabel)
					m_calibStatusLabel->setText(adcStr);
				if(m_inputVoltageLabel)
					m_inputVoltageLabel->setText(adcStr);
				onReadVshift();
				onReadVsensor();
			},
			Qt::QueuedConnection);
	});
}

void CN0540::updateVoltages()
{
	if(!m_voltMonDev)
		return;

	const char *devName = iio_device_get_name(m_voltMonDev);
	bool isXadc = devName && (strcmp(devName, "xadc") == 0);

	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		if(!m_analogIn[i] || !m_voltMonLabels[i])
			continue;
		double result = getVoltage(m_analogIn[i]);
		if(isXadc)
			result *= XADC_VREF;
		m_voltMonLabels[i]->setText(QString::number(result, 'f', 2));
	}
}
