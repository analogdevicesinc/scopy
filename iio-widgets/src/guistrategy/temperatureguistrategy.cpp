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

#include <guistrategy/temperatureguistrategy.h>

#include <QLoggingCategory>
#include <style.h>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_TEMPERATUREGUISTRATEGY, "TemperatureGuiStrategy")

using namespace scopy;

TemperatureGuiStrategy::TemperatureGuiStrategy(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent)
	: QObject(parent)
	, m_ui(new QWidget(parent))
	, m_recipe(recipe)
	, m_titleLabel(nullptr)
	, m_temperatureLabel(nullptr)
	, m_unitCombo(nullptr)
	, m_infoIcon(nullptr)
	, m_mainLayout(nullptr)
	, m_updateTimer(nullptr)
	, m_warningEnabled(false)
	, m_criticalTempCelsius(100.0)
	, m_warningOffsetCelsius(5.0)
	, m_warningMessage("Critical temperature reached!")
	, m_warningState(WarningState::Normal)
	, m_lastTemperatureCelsius(0.0)
	, m_currentUnit(TemperatureUnit::Celsius)
{
	setupUI();

	// Setup periodic updates
	m_updateTimer = new QTimer(this);
	connect(m_updateTimer, &QTimer::timeout, this, &TemperatureGuiStrategy::onPeriodicUpdate);
	setPeriodicUpdates(true, 5); // Default: enabled with 5 second interval

	// Emit initial request for data
	Q_EMIT requestData();

	qDebug(CAT_TEMPERATUREGUISTRATEGY) << "TemperatureGuiStrategy created for" << recipe.data;
}

TemperatureGuiStrategy::~TemperatureGuiStrategy()
{
	if(m_updateTimer) {
		m_updateTimer->stop();
	}
}

void TemperatureGuiStrategy::setupUI()
{
	// Create main horizontal layout
	m_mainLayout = new QHBoxLayout(m_ui);
	m_mainLayout->setContentsMargins(5, 5, 5, 5);
	m_mainLayout->setSpacing(10);

	// Title label (use recipe data)
	m_titleLabel = new QLabel(m_recipe.data + ":");
	Style::setStyle(m_titleLabel, style::properties::label::menuSmall);

	// Temperature display label
	m_temperatureLabel = new QLabel("N/A");
	Style::setStyle(m_temperatureLabel, style::properties::label::menuSmall);

	// Unit dropdown
	m_unitCombo = new QComboBox();
	m_unitCombo->addItems({"°C", "°F", "K"});
	m_unitCombo->setCurrentIndex(0); // Default to Celsius
	m_unitCombo->setMinimumWidth(50);
	m_unitCombo->setMaximumWidth(60);

	// Info icon (for GuiStrategyInterface)
	m_infoIcon = new InfoIconWidget("", m_ui);

	// Layout components
	m_mainLayout->addWidget(m_titleLabel);
	m_mainLayout->addWidget(m_infoIcon);
	m_mainLayout->addWidget(m_temperatureLabel);
	m_mainLayout->addWidget(m_unitCombo);
	m_mainLayout->addStretch();

	// Connect signals
	connect(m_unitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&TemperatureGuiStrategy::onUnitChanged);
}

void TemperatureGuiStrategy::updateUI()
{
	// Update temperature display
	if(m_temperatureLabel && m_lastTemperatureCelsius > -273.15) {
		// Convert temperature to current unit
		double convertedTemp = convertTemperature(m_lastTemperatureCelsius, m_currentUnit);

		// Format and display with unit
		QString displayText = QString::number(convertedTemp, 'f', 2);
		m_temperatureLabel->setText(displayText);
	} else if(m_temperatureLabel) {
		m_temperatureLabel->setText("N/A");
	}

	// Update warning icon
	updateInfoIconForWarning();
}

void TemperatureGuiStrategy::setCriticalTemperature(double tempCelsius, const QString &message)
{
	m_criticalTempCelsius = tempCelsius;
	m_warningMessage = message;
	m_warningEnabled = true;

	qInfo(CAT_TEMPERATUREGUISTRATEGY)
		<< "Critical temperature set to" << tempCelsius << "°C with message:" << message;

	// Update UI to show monitoring is active
	updateUI();

	// Check current temperature against new threshold
	if(m_lastTemperatureCelsius > 0) {
		updateWarningState(m_lastTemperatureCelsius);
	}
}

void TemperatureGuiStrategy::clearWarning()
{
	m_warningEnabled = false;
	m_warningState = WarningState::Normal;
	updateUI();

	qDebug(CAT_TEMPERATUREGUISTRATEGY) << "Temperature warning cleared";
}

void TemperatureGuiStrategy::setPeriodicUpdates(bool enabled, int intervalSeconds)
{
	if(!m_updateTimer) {
		return;
	}

	if(enabled) {
		m_updateTimer->setInterval(intervalSeconds * 1000);
		if(!m_updateTimer->isActive()) {
			m_updateTimer->start();
		}
		qDebug(CAT_TEMPERATUREGUISTRATEGY)
			<< "Periodic temperature updates enabled with " << intervalSeconds << " second interval";
	} else {
		if(m_updateTimer->isActive()) {
			m_updateTimer->stop();
		}
		qDebug(CAT_TEMPERATUREGUISTRATEGY) << "Periodic temperature updates disabled";
	}
}

void TemperatureGuiStrategy::setWarningOffset(double offsetCelsius)
{
	m_warningOffsetCelsius = offsetCelsius;
	qDebug(CAT_TEMPERATUREGUISTRATEGY) << "Warning offset set to" << offsetCelsius << "°C before critical";

	// Update UI to reflect new threshold
	if(m_warningEnabled) {
		updateUI();
	}
}

void TemperatureGuiStrategy::onUnitChanged()
{
	TemperatureUnit newUnit = static_cast<TemperatureUnit>(m_unitCombo->currentIndex());

	if(newUnit != m_currentUnit) {
		m_currentUnit = newUnit;

		// Trigger update to refresh display with new unit
		updateUI();

		qDebug(CAT_TEMPERATUREGUISTRATEGY) << "Unit changed to" << m_unitCombo->currentText();
	}
}

void TemperatureGuiStrategy::onPeriodicUpdate()
{
	// Trigger IIO read by requesting data through the proper IIOWidget channels
	Q_EMIT requestData();
}

void TemperatureGuiStrategy::updateWarningState(double celsius)
{
	if(!m_warningEnabled) {
		return;
	}

	double warningThreshold = m_criticalTempCelsius - m_warningOffsetCelsius;
	WarningState newState;

	// Determine new state
	if(celsius >= m_criticalTempCelsius) {
		newState = WarningState::Critical;
	} else if(celsius >= warningThreshold) {
		newState = WarningState::Approaching;
	} else {
		newState = WarningState::Normal;
	}

	// Only update if state changed
	if(newState != m_warningState) {
		WarningState oldState = m_warningState;
		m_warningState = newState;

		// Handle state transitions
		switch(newState) {
		case WarningState::Critical:
			Q_EMIT criticalTemperatureReached(celsius, m_warningMessage);
			qWarning(CAT_TEMPERATUREGUISTRATEGY) << "Critical temperature reached:" << celsius
							     << "°C (threshold:" << m_criticalTempCelsius << "°C)";
			break;
		case WarningState::Approaching:
			Q_EMIT approachingCriticalTemperature(celsius, warningThreshold, m_criticalTempCelsius);
			qWarning(CAT_TEMPERATUREGUISTRATEGY) << "Temperature approaching critical:" << celsius
							     << "°C (warning at:" << warningThreshold
							     << "°C, critical at:" << m_criticalTempCelsius << "°C)";
			break;
		case WarningState::Normal:
			if(oldState != WarningState::Normal) {
				qInfo(CAT_TEMPERATUREGUISTRATEGY)
					<< "Temperature returned to normal:" << celsius << "°C";
			}
			break;
		}

		updateInfoIconForWarning();
	}
}

void TemperatureGuiStrategy::updateInfoIconForWarning()
{
	if(!m_infoIcon) {
		return;
	}

	if(!m_warningEnabled) {
		// No warning enabled, hide icon
		m_infoIcon->setInfoMessage("");
		m_infoIcon->setStyleSheet("");
		return;
	}

	QString info_icon_path = ":/gui/icons/info.svg";
	switch(m_warningState) {
	case WarningState::Critical:
		// Critical temperature reached - RED icon
		{
			m_infoIcon->setInfoMessage(m_warningMessage);
			auto pixmap = Style::getPixmap(info_icon_path, Style::getAttribute(json::theme::content_error));
			m_infoIcon->infoIcon()->setPixmap(pixmap);
		}
		break;
	case WarningState::Approaching: {
		// Approaching critical temperature - ORANGE/YELLOW icon
		QString approachingMessage =
			QString("Warning: Approaching critical temperature! Will shut down at %1°C")
				.arg(m_criticalTempCelsius);
		m_infoIcon->setInfoMessage(approachingMessage);
		auto pixmap = Style::getPixmap(info_icon_path, Style::getAttribute(json::theme::content_busy));
		m_infoIcon->infoIcon()->setPixmap(pixmap);
	} break;
	case WarningState::Normal:
		// Normal monitoring - default color
		{
			QString shutdownMessage =
				QString("This device will shut down if reaching %1°C").arg(m_criticalTempCelsius);
			m_infoIcon->setInfoMessage(shutdownMessage);
			auto pixmap =
				Style::getPixmap(info_icon_path, Style::getAttribute(json::theme::content_default));
			m_infoIcon->infoIcon()->setPixmap(pixmap);
		}
		break;
	}
}

double TemperatureGuiStrategy::convertTemperature(double celsius, TemperatureUnit toUnit) const
{
	if(toUnit == TemperatureUnit::Fahrenheit) {
		return (celsius * 9.0 / 5.0) + 32.0;
	} else if(toUnit == TemperatureUnit::Kelvin) {
		return celsius + 273.15;
	}
	return celsius; // Celsius
}

double TemperatureGuiStrategy::parseTemperatureData(const QString &data)
{
	// Strip units from raw IIO data (e.g., "68000" or "68000 mC")
	QString valueStr = data.split(" ").first();
	bool ok;
	double rawValue = valueStr.toDouble(&ok);

	if(ok) {
		// Convert milli-Celsius to Celsius (typical for IIO temperature sensors)
		double celsius = rawValue / 1000.0;
		qDebug(CAT_TEMPERATUREGUISTRATEGY) << "Parsed temperature:" << celsius << "°C (raw:" << rawValue << ")";
		return celsius;
	} else {
		qWarning(CAT_TEMPERATUREGUISTRATEGY) << "Failed to parse temperature value:" << valueStr;
		return -999.0;
	}
}

// GuiStrategyInterface implementation
QWidget *TemperatureGuiStrategy::ui() { return m_ui; }

bool TemperatureGuiStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.iioDataOptions != "") {
		return true;
	}
	return false;
}

void TemperatureGuiStrategy::setCustomTitle(QString title)
{
	if(m_titleLabel) {
		m_titleLabel->setText(title + ":");
	}
}

void TemperatureGuiStrategy::setInfoMessage(QString infoMessage)
{
	if(m_infoIcon) {
		m_infoIcon->setInfoMessage(infoMessage);
	}
}

void TemperatureGuiStrategy::receiveData(QString currentData, QString optionalData)
{
	// Parse IIO temperature data
	double celsius = parseTemperatureData(currentData);

	if(celsius > -273.15) { // Valid temperature reading
		m_lastTemperatureCelsius = celsius;
		updateWarningState(celsius);
		updateUI();

		// Emit for any listeners
		Q_EMIT displayedNewData(currentData, optionalData);
		Q_EMIT temperatureUpdated(celsius, m_currentUnit);
	}
}

void TemperatureGuiStrategy::enableMonitoring(bool enabled)
{
	if(enabled) {
		setPeriodicUpdates(true, 5); // Default 5 second interval
		qInfo(CAT_TEMPERATUREGUISTRATEGY) << "Temperature monitoring enabled";
	} else {
		setPeriodicUpdates(false);
		clearWarning();
		qInfo(CAT_TEMPERATUREGUISTRATEGY) << "Temperature monitoring disabled";
	}
}

#include "moc_temperatureguistrategy.cpp"
