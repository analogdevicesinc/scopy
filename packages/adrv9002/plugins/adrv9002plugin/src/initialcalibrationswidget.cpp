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

#include "initialcalibrationswidget.h"

#include <QLoggingCategory>
#include <iio-widgets/iiowidgetbuilder.h>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_INITIALCALIBRATIONSWIDGET, "InitialCalibrationsWidget")

using namespace scopy::adrv9002;
using namespace scopy;

InitialCalibrationsWidget::InitialCalibrationsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_titleLabel(nullptr)
	, m_modeLabel(nullptr)
	, m_modeComboWidget(nullptr)
	, m_calibrateBtn(nullptr)
{
	setupUI();
	connectSignals();
}

InitialCalibrationsWidget::~InitialCalibrationsWidget() {}

bool InitialCalibrationsWidget::isSupported(iio_device *device)
{
	if (!device) return false;

	// Check if device has initial_calibrations_available attribute
	char buffer[256];
	int ret = iio_device_attr_read(device, "initial_calibrations_available",
	                              buffer, sizeof(buffer));
	return (ret > 0);
}

void InitialCalibrationsWidget::setupUI()
{
	// Create main widget with styling like other ADRV9002 sections
	Style::setBackgroundColor(this, json::theme::background_primary);
	Style::setStyle(this, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(15, 15, 15, 15);
	mainLayout->setSpacing(10);

	// Title
	m_titleLabel = new QLabel("Initial Calibrations", this);
	Style::setStyle(m_titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(m_titleLabel);

	// Controls layout (horizontal)
	QHBoxLayout *controlsLayout = new QHBoxLayout();
	controlsLayout->setSpacing(10);

	// Calibrations Mode label
	m_modeLabel = new QLabel("Calibrations Mode:", this);
	Style::setStyle(m_modeLabel, style::properties::label::menuSmall);
	controlsLayout->addWidget(m_modeLabel);

	// Create calibration mode combo widget with custom options (off, auto only)
	if (m_device) {
		m_modeComboWidget = IIOWidgetBuilder(this)
					    .device(m_device)
					    .attribute("initial_calibrations")
					    .optionsValues("off auto")  // Custom options - space separated string
					    .title("")  // No title as we have a separate label
					    .uiStrategy(IIOWidgetBuilder::ComboUi)
					    .infoMessage("off: Initial calibrations won't run automatically.\n"
					               "auto: Initial calibrations will run automatically for "
					               "Carrier changes bigger or equal to 100MHz.\n\n"
					               "To manually run the calibrations, press the \"Calibrate now\" button!")
					    .buildSingle();

		if (m_modeComboWidget) {
			controlsLayout->addWidget(m_modeComboWidget);
		}
	}

	// Calibrate Now button
	m_calibrateBtn = new QPushButton("Calibrate Now", this);
	m_calibrateBtn->setToolTip("Manually run initial calibrations");
	Style::setStyle(m_calibrateBtn, style::properties::button::basicButton);
	controlsLayout->addWidget(m_calibrateBtn);

	// Add stretch to push controls to the left
	controlsLayout->addStretch();

	mainLayout->addLayout(controlsLayout);
}

void InitialCalibrationsWidget::connectSignals()
{
	// Connect calibrate button
	if (m_calibrateBtn) {
		connect(m_calibrateBtn, &QPushButton::clicked,
		        this, &InitialCalibrationsWidget::onCalibrateNowClicked);
	}
}

void InitialCalibrationsWidget::onCalibrateNowClicked()
{
	if (!m_device) {
		Q_EMIT calibrationError("Device not available");
		return;
	}

	qDebug(CAT_INITIALCALIBRATIONSWIDGET) << "Starting manual calibration";

	// Write "run" to initial_calibrations attribute (like iio-oscilloscope)
	bool success = writeAttributeValue("initial_calibrations", "run");

	if (success) {
		StatusBarManager::pushMessage("Initial Calibrations started", 3000);
		Q_EMIT calibrationStarted();
		qInfo(CAT_INITIALCALIBRATIONSWIDGET) << "Manual calibration started successfully";
	} else {
		QString errorMsg = "Failed to run Initial Calibrations";
		QMessageBox::critical(this, "Initial Calibrations", errorMsg);
		Q_EMIT calibrationError(errorMsg);
		qWarning(CAT_INITIALCALIBRATIONSWIDGET) << errorMsg;
	}
}

void InitialCalibrationsWidget::refreshStatus()
{
	// Refresh the IIO widget if available
	if (m_modeComboWidget) {
		m_modeComboWidget->readAsync();
	}
}



QString InitialCalibrationsWidget::getAttributeValue(const QString &attributeName)
{
	if (!m_device) return "";

	char buffer[256];
	int ret = iio_device_attr_read(m_device, attributeName.toUtf8().constData(),
	                              buffer, sizeof(buffer));

	if (ret < 0) {
		qWarning(CAT_INITIALCALIBRATIONSWIDGET) << "Failed to read attribute" << attributeName << ":" << ret;
		return "";
	}

	return QString(buffer).trimmed();
}

bool InitialCalibrationsWidget::writeAttributeValue(const QString &attributeName, const QString &value)
{
	if (!m_device) return false;

	QByteArray valueBytes = value.toUtf8();
	int ret = iio_device_attr_write(m_device, attributeName.toUtf8().constData(),
	                               valueBytes.constData());

	if (ret < 0) {
		qWarning(CAT_INITIALCALIBRATIONSWIDGET) << "Failed to write attribute" << attributeName
		                                       << "with value" << value << ":" << ret;
		return false;
	}

	return true;
}