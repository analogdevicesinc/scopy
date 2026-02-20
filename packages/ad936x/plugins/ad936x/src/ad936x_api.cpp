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
 *
 */

#include "ad936x_api.h"
#include "ad936xplugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_AD936X_API, "AD936X_API")

using namespace scopy::ad936x;

AD936X_API::AD936X_API(Ad936xPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

AD936X_API::~AD936X_API() {}

// --- Private helpers ---

QString AD936X_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD936X_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD936X_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void AD936X_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD936X_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD936X_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList AD936X_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Global device settings ---

QString AD936X_API::getEnsmMode() { return readFromWidget("ad9361-phy/ensm_mode"); }

void AD936X_API::setEnsmMode(const QString &mode) { writeToWidget("ad9361-phy/ensm_mode", mode); }

QString AD936X_API::getCalibMode() { return readFromWidget("ad9361-phy/calib_mode"); }

void AD936X_API::setCalibMode(const QString &mode) { writeToWidget("ad9361-phy/calib_mode", mode); }

QString AD936X_API::getTrxRateGovernor() { return readFromWidget("ad9361-phy/trx_rate_governor"); }

void AD936X_API::setTrxRateGovernor(const QString &governor)
{
	writeToWidget("ad9361-phy/trx_rate_governor", governor);
}

QString AD936X_API::getRxPathRates() { return readFromWidget("ad9361-phy/rx_path_rates"); }

QString AD936X_API::getTxPathRates() { return readFromWidget("ad9361-phy/tx_path_rates"); }

QString AD936X_API::getXoCorrection() { return readFromWidget("ad9361-phy/xo_correction"); }

void AD936X_API::setXoCorrection(const QString &value) { writeToWidget("ad9361-phy/xo_correction", value); }

// --- RX chain ---

QString AD936X_API::getRxRfBandwidth() { return readFromWidget("ad9361-phy/voltage0_in/rf_bandwidth"); }

void AD936X_API::setRxRfBandwidth(const QString &value) { writeToWidget("ad9361-phy/voltage0_in/rf_bandwidth", value); }

QString AD936X_API::getRxSamplingFrequency() { return readFromWidget("ad9361-phy/voltage0_in/sampling_frequency"); }

void AD936X_API::setRxSamplingFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/sampling_frequency", value);
}

QString AD936X_API::getRxRfPortSelect() { return readFromWidget("ad9361-phy/voltage0_in/rf_port_select"); }

void AD936X_API::setRxRfPortSelect(const QString &port)
{
	writeToWidget("ad9361-phy/voltage0_in/rf_port_select", port);
}

QString AD936X_API::getRxLoFrequency() { return readFromWidget("ad9361-phy/altvoltage0_out/frequency"); }

void AD936X_API::setRxLoFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/altvoltage0_out/frequency", value);
}

QString AD936X_API::getRxHardwareGain(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9361-phy/" + chn + "_in/hardwaregain");
}

void AD936X_API::setRxHardwareGain(int channel, const QString &value)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9361-phy/" + chn + "_in/hardwaregain", value);
}

QString AD936X_API::getRxGainControlMode(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9361-phy/" + chn + "_in/gain_control_mode");
}

void AD936X_API::setRxGainControlMode(int channel, const QString &mode)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9361-phy/" + chn + "_in/gain_control_mode", mode);
}

QString AD936X_API::getRxRssi(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9361-phy/" + chn + "_in/rssi");
}

QString AD936X_API::isQuadratureTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/quadrature_tracking_en");
}

void AD936X_API::setQuadratureTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/quadrature_tracking_en", value);
}

QString AD936X_API::isRfDcOffsetTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/rf_dc_offset_tracking_en");
}

void AD936X_API::setRfDcOffsetTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/rf_dc_offset_tracking_en", value);
}

QString AD936X_API::isBbDcOffsetTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/bb_dc_offset_tracking_en");
}

void AD936X_API::setBbDcOffsetTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/bb_dc_offset_tracking_en", value);
}

// --- TX chain ---

QString AD936X_API::getTxRfBandwidth() { return readFromWidget("ad9361-phy/voltage0_out/rf_bandwidth"); }

void AD936X_API::setTxRfBandwidth(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_out/rf_bandwidth", value);
}

QString AD936X_API::getTxSamplingFrequency() { return readFromWidget("ad9361-phy/voltage0_out/sampling_frequency"); }

void AD936X_API::setTxSamplingFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_out/sampling_frequency", value);
}

QString AD936X_API::getTxRfPortSelect() { return readFromWidget("ad9361-phy/voltage0_out/rf_port_select"); }

void AD936X_API::setTxRfPortSelect(const QString &port)
{
	writeToWidget("ad9361-phy/voltage0_out/rf_port_select", port);
}

QString AD936X_API::getTxLoFrequency() { return readFromWidget("ad9361-phy/altvoltage1_out/frequency"); }

void AD936X_API::setTxLoFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/altvoltage1_out/frequency", value);
}

QString AD936X_API::getTxHardwareGain(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9361-phy/" + chn + "_out/hardwaregain");
}

void AD936X_API::setTxHardwareGain(int channel, const QString &value)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9361-phy/" + chn + "_out/hardwaregain", value);
}

QString AD936X_API::getTxRssi(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9361-phy/" + chn + "_out/rssi");
}

// --- Generic widget access ---

QStringList AD936X_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString AD936X_API::readWidget(const QString &key) { return readFromWidget(key); }

void AD936X_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void AD936X_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

bool AD936X_API::isFmcomms5() { return m_plugin->m_isFmcomms5; }

#include "moc_ad936x_api.cpp"
