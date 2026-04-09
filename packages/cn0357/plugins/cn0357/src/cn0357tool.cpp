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

#include "cn0357tool.h"

#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QScrollArea>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QtConcurrentRun>
#include <gui/widgets/menusectionwidget.h>
#include <style.h>

#include "cn0357widgetfactory.h"

Q_LOGGING_CATEGORY(CAT_CN0357TOOL, "Cn0357Tool")
using namespace scopy::cn0357;

Cn0357Tool::Cn0357Tool(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
	, m_tool(nullptr)
	, m_refreshButton(nullptr)
	, m_updateTimer(nullptr)
{
	findDevicesAndChannels();
	setupUi();
}

void Cn0357Tool::findDevicesAndChannels()
{
	if(!m_ctx)
		return;

	m_adcDev = iio_context_find_device(m_ctx, "ad7790");
	iio_device *dpotDev = iio_context_find_device(m_ctx, "ad5270_iio");

	if(m_adcDev) {
		m_adcCh = iio_device_find_channel(m_adcDev, "voltage0", false);
		m_pwrCh = iio_device_find_channel(m_adcDev, "supply", false);
	}

	if(dpotDev) {
		m_rdacCh = iio_device_find_channel(dpotDev, "voltage0", true);
	}

	if(!m_adcDev)
		qWarning(CAT_CN0357TOOL) << "ad7790 ADC device not found";
	if(!dpotDev)
		qWarning(CAT_CN0357TOOL) << "ad5270_iio DPOT not found (optional)";
}

void Cn0357Tool::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();
		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);
		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });
		watcher->setFuture(future);
	});

	QWidget *controlsScrollWidget = new QWidget(this);
	QVBoxLayout *controlsScrollLayout = new QVBoxLayout(controlsScrollWidget);
	controlsScrollLayout->setContentsMargins(0, 0, 0, 0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsScrollWidget);

	m_tool->addWidgetToCentralContainerHelper(scrollArea);

	if(!m_ctx) {
		qWarning(CAT_CN0357TOOL) << "No context provided";
		return;
	}

	controlsScrollLayout->addWidget(createAdcSettingsSection(controlsScrollWidget));
	controlsScrollLayout->addWidget(createFeedbackSettingsSection(controlsScrollWidget));
	controlsScrollLayout->addWidget(createSystemSection(controlsScrollWidget));
	controlsScrollLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	// Start 1s polling timer for ADC readings
	m_updateTimer = new QTimer(this);
	m_updateTimer->setInterval(1000);
	connect(m_updateTimer, &QTimer::timeout, this, [this]() {
		if(!isVisible())
			return;
		if(m_adcMvWidget)
			m_adcMvWidget->readAsync();
		if(m_supplyVWidget)
			m_supplyVWidget->readAsync();
	});
	m_updateTimer->start();

	// Trigger initial coefficient display
	updateCoefficients();
}

QWidget *Cn0357Tool::createAdcSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"ADC Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	IIOWidget *samplingFreqWidget = Cn0357WidgetFactory::createComboWidget(
		m_adcDev, "sampling_frequency", "sampling_frequency_available", "Update Rate (Hz)", m_group, section);
	if(samplingFreqWidget) {
		section->contentLayout()->addWidget(samplingFreqWidget);
		connect(this, &Cn0357Tool::readRequested, samplingFreqWidget, &IIOWidget::readAsync);
	}

	return section;
}

QWidget *Cn0357Tool::createFeedbackSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"Feedback Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// Feedback type combo (UI-only, not an IIO attribute)
	m_feedbackType = new QComboBox(section);
	m_feedbackType->addItem("Rheostat");
	m_feedbackType->addItem("Fixed Resistor");
	section->contentLayout()->addWidget(new QLabel("Feedback Type", section));
	section->contentLayout()->addWidget(m_feedbackType);

	// RDAC raw value spinbox (0-1023, default 307)
	m_rdacSpinBox = new gui::MenuSpinbox("RDAC Value", 307, "", 0, 1023, true, false, false, section);
	m_rdacSpinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_rdacSpinBox->setScalingEnabled(false);
	section->contentLayout()->addWidget(m_rdacSpinBox);
	connect(m_rdacSpinBox, &gui::MenuSpinbox::valueChanged, this, &Cn0357Tool::onRdacSpinChanged);

	// Program Rheostat button — explicitly writes current RDAC value to IIO
	m_programRdacBtn = new QPushButton("Program Rheostat", section);
	Style::setStyle(m_programRdacBtn, style::properties::button::basicButton);
	section->contentLayout()->addWidget(m_programRdacBtn);
	connect(m_programRdacBtn, &QPushButton::clicked, this, [this]() {
		if(!m_rdacCh)
			return;
		QString raw = QString::number((int)m_rdacSpinBox->value());
		iio_channel_attr_write(m_rdacCh, "raw", raw.toStdString().c_str());
	});

	if(!m_rdacCh) {
		// DPOT not found — disable rheostat controls and default to Fixed Resistor
		m_rdacSpinBox->setEnabled(false);
		m_programRdacBtn->setEnabled(false);
		m_feedbackType->setCurrentIndex(1);
	}

	// Initialize feedback resistance from default RDAC value
	m_feedbackResistance = 307.0 / 1024.0 * 20000.0;

	// Fixed resistor spinbox (hidden by default — shown in Fixed Resistor mode)
	m_fixedRes = new gui::MenuSpinbox("Fixed Resistor", 0, "Ω", 0, 1e9, true, false, false, section);
	m_fixedRes->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_fixedRes->setScalingEnabled(false);
	m_fixedRes->setVisible(false);
	m_fixedResLabel = new QLabel("Fixed Resistor (Ω)", section);
	m_fixedResLabel->setVisible(false);
	section->contentLayout()->addWidget(m_fixedResLabel);
	section->contentLayout()->addWidget(m_fixedRes);

	// Wire feedback type toggle
	connect(m_feedbackType, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&Cn0357Tool::onFeedbackTypeChanged);

	// Wire fixed resistor value changes
	connect(m_fixedRes, &gui::MenuSpinbox::valueChanged, this, &Cn0357Tool::onFixedResistorChanged);

	return section;
}

QWidget *Cn0357Tool::createSystemSection(QWidget *parent)
{
	// System section: Measurements (left) + Data (right) side by side

	MenuSectionCollapseWidget *systemWidget = new MenuSectionCollapseWidget(
		"System", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *content = new QWidget(systemWidget);
	QHBoxLayout *layout = new QHBoxLayout(content);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	// --- Left: Measurements ---
	QWidget *measurementsWidget = new QWidget(content);
	QVBoxLayout *measurementsLayout = new QVBoxLayout(measurementsWidget);
	measurementsLayout->setContentsMargins(0, 0, 0, 0);
	measurementsLayout->setSpacing(10);

	QLabel *measurementsTitle = new QLabel("Measurements", measurementsWidget);
	Style::setStyle(measurementsTitle, style::properties::label::menuBig);
	measurementsLayout->addWidget(measurementsTitle);

	// Concentration (ppm) — computed, no IIO backing
	measurementsLayout->addWidget(new QLabel("Concentration (ppm)", measurementsWidget));
	m_ppmDisplay = new QLineEdit(measurementsWidget);
	m_ppmDisplay->setReadOnly(true);
	m_ppmDisplay->setPlaceholderText("–");
	measurementsLayout->addWidget(m_ppmDisplay);

	// Conversion (mV) — IIOWidget ReadOnly, adc voltage0-voltage0, raw→mV
	// Factory handles null channel: returns disabled IIOWidget with info message
	m_adcMvWidget = Cn0357WidgetFactory::createAdcReadOnlyWidget(m_adcDev, m_adcCh, "raw", "Conversion (mV)",
								     1200.0, m_group, measurementsWidget);
	if(m_adcMvWidget) {
		measurementsLayout->addWidget(m_adcMvWidget);
		connect(this, &Cn0357Tool::readRequested, m_adcMvWidget, &IIOWidget::readAsync);

		// Connect to data strategy's sendData to intercept raw ADC reads for ppm computation
		QObject *dsObj = dynamic_cast<QObject *>(m_adcMvWidget->getDataStrategy());
		if(dsObj) {
			connect(dsObj, SIGNAL(sendData(QString, QString)), this, SLOT(onAdcMvChanged(QString)));
		}
	}

	// Supply Voltage (V) — IIOWidget ReadOnly, adc supply, raw→V
	// Factory handles null channel: returns disabled IIOWidget with info message
	m_supplyVWidget = Cn0357WidgetFactory::createAdcReadOnlyWidget(m_adcDev, m_pwrCh, "raw", "Supply Voltage (V)",
								       5.85, m_group, measurementsWidget);
	if(m_supplyVWidget) {
		measurementsLayout->addWidget(m_supplyVWidget);
		connect(this, &Cn0357Tool::readRequested, m_supplyVWidget, &IIOWidget::readAsync);
	}

	measurementsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	// --- Right: Data ---
	QWidget *dataWidget = new QWidget(content);

	QVBoxLayout *dataLayout = new QVBoxLayout(dataWidget);
	dataLayout->setContentsMargins(0, 0, 0, 0);
	dataLayout->setSpacing(10);

	QLabel *dataTitle = new QLabel("Data", dataWidget);
	Style::setStyle(dataTitle, style::properties::label::menuBig);
	dataLayout->addWidget(dataTitle);

	// Sensor Sensitivity (nA/ppm) — user input, drives ppm calc
	m_sensorSensitivity = new gui::MenuSpinbox("Sensor Sensitivity (nA/ppm)", 65, "nA/ppm", 1, 100, true, false,
						   false, dataWidget);
	m_sensorSensitivity->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_sensorSensitivity->setScalingEnabled(false);
	dataLayout->addWidget(m_sensorSensitivity);

	// Feedback Resistance (Ω) — computed, read-only
	dataLayout->addWidget(new QLabel("Feedback Resistance (Ω)", dataWidget));
	m_feedbackResDisplay = new QLineEdit(dataWidget);
	m_feedbackResDisplay->setReadOnly(true);
	dataLayout->addWidget(m_feedbackResDisplay);

	// ppm/mV coefficient — computed, read-only
	dataLayout->addWidget(new QLabel("ppm/mV", dataWidget));
	m_ppmMvDisplay = new QLineEdit(dataWidget);
	m_ppmMvDisplay->setReadOnly(true);
	dataLayout->addWidget(m_ppmMvDisplay);

	// mV/ppm coefficient — computed, read-only
	dataLayout->addWidget(new QLabel("mV/ppm", dataWidget));
	m_mvPpmDisplay = new QLineEdit(dataWidget);
	m_mvPpmDisplay->setReadOnly(true);
	dataLayout->addWidget(m_mvPpmDisplay);

	connect(m_sensorSensitivity, &gui::MenuSpinbox::valueChanged, this, &Cn0357Tool::onSensorSensitivityChanged);

	dataLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	layout->addWidget(measurementsWidget);
	layout->addWidget(dataWidget);

	systemWidget->contentLayout()->addWidget(content);

	return systemWidget;
}

void Cn0357Tool::onFeedbackTypeChanged(int index)
{
	bool isRheostat = (index == 0);

	if(m_rdacSpinBox)
		m_rdacSpinBox->setVisible(isRheostat);
	if(m_fixedRes)
		m_fixedRes->setVisible(!isRheostat);
	if(m_fixedResLabel)
		m_fixedResLabel->setVisible(!isRheostat);

	if(m_programRdacBtn)
		m_programRdacBtn->setVisible(isRheostat);

	if(isRheostat) {
		if(m_rdacSpinBox)
			onRdacSpinChanged(m_rdacSpinBox->value());
	} else {
		if(m_fixedRes) {
			m_feedbackResistance = m_fixedRes->value();
			updateCoefficients();
		}
	}
}

void Cn0357Tool::onRdacSpinChanged(double raw)
{
	if(m_feedbackType && m_feedbackType->currentIndex() != 0)
		return;

	m_feedbackResistance = raw / 1024.0 * 20000.0;
	updateCoefficients();
}

void Cn0357Tool::onFixedResistorChanged(double value)
{
	if(m_feedbackType && m_feedbackType->currentIndex() != 1)
		return;

	m_feedbackResistance = value;
	updateCoefficients();
}

void Cn0357Tool::onSensorSensitivityChanged(double value)
{
	m_sensorSens = value;
	updateCoefficients();
}

void Cn0357Tool::onAdcMvChanged(QString value)
{
	// value is raw integer from IIO (data strategy sendData fires before dataToUIConversion)
	// Apply same conversion as factory: (raw/32768 - 1) * 1200 = mV
	bool ok;
	double raw = value.toDouble(&ok);
	if(!ok)
		return;

	double adcMv = (raw / 32768.0 - 1.0) * 1200.0;
	m_adcMv = adcMv;

	double mvPpm = m_sensorSens * m_feedbackResistance * 1e-6;
	if(mvPpm <= 0.0) {
		if(m_ppmDisplay)
			m_ppmDisplay->setText("–");
		return;
	}

	double ppmMv = 1.0 / mvPpm;
	double ppm = qMax(0.0, adcMv * ppmMv);

	if(m_ppmDisplay)
		m_ppmDisplay->setText(QString::number(ppm, 'f', 3));
}

void Cn0357Tool::updateCoefficients()
{
	double mvPpm = m_sensorSens * m_feedbackResistance * 1e-6;

	if(m_feedbackResDisplay)
		m_feedbackResDisplay->setText(QString::number(m_feedbackResistance, 'f', 2));

	if(m_mvPpmDisplay)
		m_mvPpmDisplay->setText(QString::number(mvPpm, 'g', 6));

	if(mvPpm > 0.0) {
		double ppmMv = 1.0 / mvPpm;
		if(m_ppmMvDisplay)
			m_ppmMvDisplay->setText(QString::number(ppmMv, 'g', 6));
	} else {
		if(m_ppmMvDisplay)
			m_ppmMvDisplay->setText("–");
	}

	// Recompute ppm with last known ADC mV value
	if(mvPpm > 0.0 && m_ppmDisplay)
		m_ppmDisplay->setText(QString::number(qMax(0.0, m_adcMv / mvPpm), 'f', 3));
	else if(m_ppmDisplay)
		m_ppmDisplay->setText("–");
}
