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

#include "cn0540_api.h"
#include "cn0540.h"
#include "cn0540plugin.h"

#include <QLoggingCategory>
#include <pluginbase/toolmenuentry.h>

Q_LOGGING_CATEGORY(CAT_CN0540_API, "CN0540_API")

// Matches the DAC_BUF_GAIN constant defined in cn0540.cpp
static constexpr double CN0540_DAC_BUF_GAIN = 1.22;

using namespace scopy::cn0540;

CN0540_API::CN0540_API(CN0540Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

CN0540_API::~CN0540_API() {}

CN0540 *CN0540_API::getTool() const
{
	if(!m_plugin || m_plugin->m_toolList.isEmpty())
		return nullptr;
	return qobject_cast<CN0540 *>(m_plugin->m_toolList[0]->tool());
}

QStringList CN0540_API::getTools()
{
	QStringList tools;
	if(!m_plugin)
		return tools;
	for(ToolMenuEntry *tme : m_plugin->m_toolList) {
		tools.append(tme->name());
	}
	return tools;
}

void CN0540_API::refresh()
{
	CN0540 *tool = getTool();
	if(!tool)
		return;
	Q_EMIT tool->readAll();
}

// --- Power Control ---

QString CN0540_API::getSwFF()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioSwFF) {
		qWarning(CAT_CN0540_API) << "SW_FF GPIO channel not available";
		return QString();
	}
	return tool->getGpioState(tool->m_gpioSwFF) ? "HIGH" : "LOW";
}

QString CN0540_API::getShutdown()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioShutdown) {
		qWarning(CAT_CN0540_API) << "Shutdown GPIO channel not available";
		return QString();
	}
	// Active-low: GPIO LOW = operating (enabled), GPIO HIGH = shutdown
	bool operating = !tool->getGpioState(tool->m_gpioShutdown);
	return operating ? "true" : "false";
}

void CN0540_API::setShutdown(const QString &enabled)
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioShutdown) {
		qWarning(CAT_CN0540_API) << "Shutdown GPIO channel not available";
		return;
	}
	bool operating = (enabled == "true" || enabled == "1");
	// Active-low: to operate (enabled=true), write GPIO LOW (false)
	tool->setGpioState(tool->m_gpioShutdown, !operating);
}

QString CN0540_API::getConstantCurrent()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioCC) {
		qWarning(CAT_CN0540_API) << "Constant current GPIO channel not available";
		return QString();
	}
	return tool->getGpioState(tool->m_gpioCC) ? "true" : "false";
}

void CN0540_API::setConstantCurrent(const QString &enabled)
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioCC) {
		qWarning(CAT_CN0540_API) << "Constant current GPIO channel not available";
		return;
	}
	tool->setGpioState(tool->m_gpioCC, enabled == "true" || enabled == "1");
}

// --- ADC Driver Settings ---

QString CN0540_API::getFdaEnabled()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioFdaDis) {
		qWarning(CAT_CN0540_API) << "FDA_DIS GPIO channel not available";
		return QString();
	}
	// Active-low: GPIO LOW = FDA enabled, GPIO HIGH = FDA disabled
	bool enabled = !tool->getGpioState(tool->m_gpioFdaDis);
	return enabled ? "true" : "false";
}

void CN0540_API::setFdaEnabled(const QString &enabled)
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioFdaDis) {
		qWarning(CAT_CN0540_API) << "FDA_DIS GPIO channel not available";
		return;
	}
	bool enable = (enabled == "true" || enabled == "1");
	// Active-low: to enable FDA (true), write GPIO LOW (false)
	tool->setGpioState(tool->m_gpioFdaDis, !enable);
}

QString CN0540_API::getFdaMode()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioFdaMode) {
		qWarning(CAT_CN0540_API) << "FDA_MODE GPIO channel not available";
		return QString();
	}
	return tool->getGpioState(tool->m_gpioFdaMode) ? "FULL POWER" : "LOW POWER";
}

void CN0540_API::setFdaMode(const QString &mode)
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_gpioFdaMode) {
		qWarning(CAT_CN0540_API) << "FDA_MODE GPIO channel not available";
		return;
	}
	static const QStringList options = {"FULL POWER", "LOW POWER"};
	if(!options.contains(mode)) {
		qWarning(CAT_CN0540_API) << "Invalid FDA mode:" << mode << "Valid options:" << options;
		return;
	}
	tool->setGpioState(tool->m_gpioFdaMode, mode == "FULL POWER");
}

// --- Sensor Calibration ---

QString CN0540_API::getInputVoltage()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_adcCh) {
		qWarning(CAT_CN0540_API) << "ADC channel not available";
		return QString();
	}
	return QString::number(tool->getVoltage(tool->m_adcCh), 'f', 4);
}

QString CN0540_API::getShiftVoltage()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_dacCh) {
		qWarning(CAT_CN0540_API) << "DAC channel not available";
		return QString();
	}
	return QString::number(tool->getVshiftMv(), 'f', 4);
}

void CN0540_API::setShiftVoltage(const QString &mV)
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_dacCh) {
		qWarning(CAT_CN0540_API) << "DAC channel not available";
		return;
	}
	bool ok;
	double vshiftMv = mV.toDouble(&ok);
	if(!ok) {
		qWarning(CAT_CN0540_API) << "Invalid shift voltage value:" << mV;
		return;
	}
	// The UI displays DAC voltage * DAC_BUF_GAIN; to set that displayed value,
	// write the raw DAC voltage = vshiftMv / DAC_BUF_GAIN
	tool->setVoltage(tool->m_dacCh, vshiftMv / CN0540_DAC_BUF_GAIN);
}

QString CN0540_API::getSensorVoltage()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_adcCh || !tool->m_dacCh) {
		qWarning(CAT_CN0540_API) << "ADC or DAC channel not available";
		return QString();
	}
	tool->onReadVsensor();
	return tool->m_sensorVoltageLabel ? tool->m_sensorVoltageLabel->text() : QString();
}

void CN0540_API::calibrate()
{
	CN0540 *tool = getTool();
	if(!tool) {
		qWarning(CAT_CN0540_API) << "CN0540 tool not available";
		return;
	}
	tool->onCalibrate();
}

// --- Voltage Monitor ---

QStringList CN0540_API::getVoltageMonitor()
{
	CN0540 *tool = getTool();
	if(!tool || !tool->m_voltMonDev)
		return QStringList();
	QStringList result;
	for(int i = 0; i < NUM_ANALOG_PINS; i++) {
		result.append(tool->m_voltMonLabels[i] ? tool->m_voltMonLabels[i]->text() : QString());
	}
	return result;
}

#include "moc_cn0540_api.cpp"
