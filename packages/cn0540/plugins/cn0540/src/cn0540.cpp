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

#include <iio-widgets/iiowidgetbuilder.h>
#include <gui/style_properties.h>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_CN0540, "CN0540")

static constexpr double G = 0.3;
static constexpr double FDA_VOCM_MV = 2500.0;
static constexpr double FDA_GAIN = 2.667;
static constexpr int CALIB_MAX_ITER = 20;
static constexpr double XADC_VREF = 3.3;

using namespace scopy;
using namespace scopy::cn0540;

CN0540::CN0540(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
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
	, m_group(group)
	, m_voltMonTimer(nullptr)
	, m_swffStatusLabel(nullptr)
	, m_sensorVoltageLabel(nullptr)
	, m_calibStatusLabel(nullptr)
	, m_vshiftLineEdit(nullptr)
	, m_shutdownWidget(nullptr)
	, m_ccWidget(nullptr)
	, m_fdaWidget(nullptr)
	, m_fdaModeWidget(nullptr)
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
	QMetaObject::invokeMethod(this, &CN0540::onCalibrate, Qt::QueuedConnection);
}

CN0540::~CN0540()
{
	if(m_voltMonTimer)
		m_voltMonTimer->stop();
	m_calibFuture.waitForFinished();
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
	m_isXadc = devName && (strcmp(devName, "xadc") == 0);
	int startIdx = m_isXadc ? 9 : 0;

	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		char name[16];
		snprintf(name, sizeof(name), "voltage%d", startIdx + i);
		m_analogIn[i] = iio_device_find_channel(m_voltMonDev, name, false);
	}
}

bool CN0540::getGpioState(iio_channel *ch)
{
	long long val = 0;
	int ret = iio_channel_attr_read_longlong(ch, "raw", &val);
	if(ret < 0)
		qWarning(CAT_CN0540) << "getGpioState: failed to read raw attr, ret=" << ret;
	return (bool)val;
}

void CN0540::setGpioState(iio_channel *ch, bool state)
{
	int ret = iio_channel_attr_write_longlong(ch, "raw", (long long)state);
	if(ret < 0)
		qWarning(CAT_CN0540) << "setGpioState: failed to write raw attr, ret=" << ret;
}

double CN0540::getVoltage(iio_channel *ch)
{
	long long raw = 0;
	double scale = 0.0;
	int ret = iio_channel_attr_read_longlong(ch, "raw", &raw);
	if(ret < 0)
		qWarning(CAT_CN0540) << "getVoltage: failed to read raw attr, ret=" << ret;
	ret = iio_channel_attr_read_double(ch, "scale", &scale);
	if(ret < 0)
		qWarning(CAT_CN0540) << "getVoltage: failed to read scale attr, ret=" << ret;
	return (double)raw * scale;
}

void CN0540::setVoltage(iio_channel *ch, double voltageMv)
{
	double scale = 0.0;
	int ret = iio_channel_attr_read_double(ch, "scale", &scale);
	if(ret < 0) {
		qWarning(CAT_CN0540) << "setVoltage: failed to read scale attr, ret=" << ret;
		return;
	}
	if(scale != 0.0)
		iio_channel_attr_write_longlong(ch, "raw", (long long)(voltageMv / scale));
}

double CN0540::getVshiftMv() { return getVoltage(m_dacCh) * DAC_BUF_GAIN; }

void CN0540::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
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
	connect(this, &CN0540::readAll, this, &CN0540::onReadVshift);
	connect(this, &CN0540::readAll, this, &CN0540::onReadVsensor);
	if(m_shutdownWidget)
		connect(this, &CN0540::readAll, m_shutdownWidget, &IIOWidget::readAsync);
	if(m_ccWidget)
		connect(this, &CN0540::readAll, m_ccWidget, &IIOWidget::readAsync);
	if(m_fdaWidget)
		connect(this, &CN0540::readAll, m_fdaWidget, &IIOWidget::readAsync);
	if(m_fdaModeWidget)
		connect(this, &CN0540::readAll, m_fdaModeWidget, &IIOWidget::readAsync);
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
	layout->addRow(swffRow);

	// SHUTDOWN row (active-low: GPIO low=operating, GPIO high=shutdown)
	if(m_gpioShutdown) {
		m_shutdownWidget = IIOWidgetBuilder(container)
					   .channel(m_gpioShutdown)
					   .attribute("raw")
					   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					   .title("Shutdown")
					   .group(m_group)
					   .buildSingle();
		m_shutdownWidget->setDataToUIConversion([](QString s) { return s == "0" ? "1" : "0"; });
		m_shutdownWidget->setUItoDataConversion([](QString s) { return s == "1" ? "0" : "1"; });
		m_shutdownWidget->showProgressBar(false);
		layout->addRow(m_shutdownWidget);
	}

	// CONSTANT CURRENT row (active-high: GPIO high=LED on=constant current)
	if(m_gpioCC) {
		m_ccWidget = IIOWidgetBuilder(container)
				     .channel(m_gpioCC)
				     .attribute("raw")
				     .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				     .title("Constant Current")
				     .group(m_group)
				     .buildSingle();
		m_ccWidget->showProgressBar(false);
		layout->addRow(m_ccWidget);
	}

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

	// FDA Status row (active-low: GPIO low=FDA enabled, GPIO high=FDA disabled)
	if(m_gpioFdaDis) {
		m_fdaWidget = IIOWidgetBuilder(container)
				      .channel(m_gpioFdaDis)
				      .attribute("raw")
				      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				      .title("FDA Status")
				      .group(m_group)
				      .buildSingle();
		m_fdaWidget->setDataToUIConversion([](QString s) { return s == "0" ? "1" : "0"; });
		m_fdaWidget->setUItoDataConversion([](QString s) { return s == "1" ? "0" : "1"; });
		m_fdaWidget->showProgressBar(false);
		layout->addRow(m_fdaWidget);
	}

	// FDA Mode row (raw=1 → FULL POWER, raw=0 → LOW POWER; maps directly to checkbox)
	if(m_gpioFdaMode) {
		m_fdaModeWidget = IIOWidgetBuilder(container)
					  .channel(m_gpioFdaMode)
					  .attribute("raw")
					  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					  .title("FDA Mode (checked = FULL POWER)")
					  .group(m_group)
					  .buildSingle();
		m_fdaModeWidget->showProgressBar(false);
		layout->addRow(m_fdaModeWidget);
	}

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

	// Row 0: Input voltage — shows calibration result; Calibrate button alongside
	layout->addWidget(new QLabel("Input Voltage [mV]:", container), 0, 0, Qt::AlignRight);
	m_calibStatusLabel = new QLabel("---", container);
	Style::setStyle(m_calibStatusLabel, style::properties::label::menuSmall);
	layout->addWidget(m_calibStatusLabel, 0, 1);
	QPushButton *calibBtn = new QPushButton("Calibrate", container);
	Style::setStyle(calibBtn, style::properties::button::basicButton);
	connect(calibBtn, &QPushButton::clicked, this, &CN0540::onCalibrate);
	layout->addWidget(calibBtn, 0, 2);

	// Row 1: Shift voltage — editable QLineEdit with explicit Read and Write buttons
	layout->addWidget(new QLabel("Shift Voltage [mV]:", container), 1, 0, Qt::AlignRight);
	m_vshiftLineEdit = new QLineEdit(container);
	m_vshiftLineEdit->setPlaceholderText("---");
	layout->addWidget(m_vshiftLineEdit, 1, 1);
	QPushButton *readVshiftBtn = new QPushButton("Read", container);
	Style::setStyle(readVshiftBtn, style::properties::button::basicButton);
	connect(readVshiftBtn, &QPushButton::clicked, this, &CN0540::onReadVshift);
	layout->addWidget(readVshiftBtn, 1, 2);
	QPushButton *writeVshiftBtn = new QPushButton("Write", container);
	Style::setStyle(writeVshiftBtn, style::properties::button::basicButton);
	connect(writeVshiftBtn, &QPushButton::clicked, this, [this]() {
		if(!m_dacCh || !m_vshiftLineEdit)
			return;
		bool ok;
		double mV = m_vshiftLineEdit->text().toDouble(&ok);
		if(ok)
			setVoltage(m_dacCh, mV / DAC_BUF_GAIN);
	});
	layout->addWidget(writeVshiftBtn, 1, 3);

	// Row 2: Sensor voltage — read-only label with explicit Read button
	layout->addWidget(new QLabel("Sensor Voltage [mV]:", container), 2, 0, Qt::AlignRight);
	m_sensorVoltageLabel = new QLabel("---", container);
	Style::setStyle(m_sensorVoltageLabel, style::properties::label::menuSmall);
	layout->addWidget(m_sensorVoltageLabel, 2, 1);
	QPushButton *readVsensorBtn = new QPushButton("Read", container);
	Style::setStyle(readVsensorBtn, style::properties::button::basicButton);
	connect(readVsensorBtn, &QPushButton::clicked, this, &CN0540::onReadVsensor);
	layout->addWidget(readVsensorBtn, 2, 2);

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

void CN0540::onReadVshift()
{
	if(!m_dacCh || !m_vshiftLineEdit)
		return;
	m_vshiftLineEdit->setText(QString::number(getVshiftMv(), 'f', 4));
}

void CN0540::onReadSwFF()
{
	if(!m_gpioSwFF)
		return;
	bool state = getGpioState(m_gpioSwFF);
	m_swffStatusLabel->setText(state ? "HIGH" : "LOW");
}

void CN0540::onReadVsensor()
{
	if(!m_adcCh || !m_dacCh)
		return;
	double vadcMv = getVoltage(m_adcCh);
	double vshiftMv = getVshiftMv();
	double v1St = FDA_VOCM_MV - vadcMv / FDA_GAIN;
	double vsensorMv = ((G + 1.0) * vshiftMv - v1St) / G;
	vsensorMv -= vadcMv;
	m_sensorVoltageLabel->setText(QString::number(vsensorMv, 'f', 4));
}

void CN0540::onCalibrate()
{
	if(!m_adcCh || !m_dacCh)
		return;

	if(m_calibFuture.isRunning())
		return;

	if(m_calibStatusLabel)
		m_calibStatusLabel->setText("Calibrating...");

	if(m_voltMonTimer)
		m_voltMonTimer->stop();

	m_calibFuture = QtConcurrent::run([this]() {
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
				onReadVshift();
				onReadVsensor();
				if(m_voltMonTimer)
					m_voltMonTimer->start(1000);
			},
			Qt::QueuedConnection);
	});
}

void CN0540::updateVoltages()
{
	if(!m_voltMonDev)
		return;

	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		if(!m_analogIn[i] || !m_voltMonLabels[i])
			continue;
		double result = getVoltage(m_analogIn[i]);
		if(m_isXadc)
			result *= XADC_VREF;
		m_voltMonLabels[i]->setText(QString::number(result, 'f', 2));
	}
}
