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

#include "fmcomms5_api.h"
#include "ad936xplugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_FMCOMMS5_API, "FMCOMMS5_API")

using namespace scopy::ad936x;

FMCOMMS5_API::FMCOMMS5_API(Ad936xPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

FMCOMMS5_API::~FMCOMMS5_API() {}

// --- Private helpers ---

QString FMCOMMS5_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS5_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_FMCOMMS5_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void FMCOMMS5_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_FMCOMMS5_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_FMCOMMS5_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

QString FMCOMMS5_API::deviceName(int device) { return (device == 0) ? "ad9361-phy" : "ad9361-phy-B"; }

QString FMCOMMS5_API::rxChannelKey(int channel, const QString &attr)
{
	// channel 0-1: ad9361-phy voltage0/voltage1 input
	// channel 2-3: ad9361-phy-B voltage0/voltage1 input
	QString dev = (channel < 2) ? "ad9361-phy" : "ad9361-phy-B";
	QString chn = (channel % 2 == 0) ? "voltage0" : "voltage1";
	return dev + "/" + chn + "_in/" + attr;
}

QString FMCOMMS5_API::txChannelKey(int channel, const QString &attr)
{
	// channel 0-1: ad9361-phy voltage0/voltage1 output
	// channel 2-3: ad9361-phy-B voltage0/voltage1 output
	QString dev = (channel < 2) ? "ad9361-phy" : "ad9361-phy-B";
	QString chn = (channel % 2 == 0) ? "voltage0" : "voltage1";
	return dev + "/" + chn + "_out/" + attr;
}

// --- Tool management ---

QStringList FMCOMMS5_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Global device settings ---

QString FMCOMMS5_API::getEnsmMode() { return readFromWidget("ad9361-phy/ensm_mode"); }

void FMCOMMS5_API::setEnsmMode(const QString &mode) { writeToWidget("ad9361-phy/ensm_mode", mode); }

QString FMCOMMS5_API::getCalibMode() { return readFromWidget("ad9361-phy/calib_mode"); }

void FMCOMMS5_API::setCalibMode(const QString &mode) { writeToWidget("ad9361-phy/calib_mode", mode); }

QString FMCOMMS5_API::getTrxRateGovernor() { return readFromWidget("ad9361-phy/trx_rate_governor"); }

void FMCOMMS5_API::setTrxRateGovernor(const QString &governor)
{
	writeToWidget("ad9361-phy/trx_rate_governor", governor);
}

QString FMCOMMS5_API::getRxPathRates() { return readFromWidget("ad9361-phy/rx_path_rates"); }

QString FMCOMMS5_API::getTxPathRates() { return readFromWidget("ad9361-phy/tx_path_rates"); }

QString FMCOMMS5_API::getXoCorrection() { return readFromWidget("ad9361-phy/xo_correction"); }

void FMCOMMS5_API::setXoCorrection(const QString &value) { writeToWidget("ad9361-phy/xo_correction", value); }

// --- RX chain settings ---

QString FMCOMMS5_API::getRxRfBandwidth() { return readFromWidget("ad9361-phy/voltage0_in/rf_bandwidth"); }

void FMCOMMS5_API::setRxRfBandwidth(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/rf_bandwidth", value);
}

QString FMCOMMS5_API::getRxSamplingFrequency() { return readFromWidget("ad9361-phy/voltage0_in/sampling_frequency"); }

void FMCOMMS5_API::setRxSamplingFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/sampling_frequency", value);
}

QString FMCOMMS5_API::getRxRfPortSelect() { return readFromWidget("ad9361-phy/voltage0_in/rf_port_select"); }

void FMCOMMS5_API::setRxRfPortSelect(const QString &port)
{
	writeToWidget("ad9361-phy/voltage0_in/rf_port_select", port);
}

QString FMCOMMS5_API::isQuadratureTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/quadrature_tracking_en");
}

void FMCOMMS5_API::setQuadratureTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/quadrature_tracking_en", value);
}

QString FMCOMMS5_API::isRfDcOffsetTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/rf_dc_offset_tracking_en");
}

void FMCOMMS5_API::setRfDcOffsetTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/rf_dc_offset_tracking_en", value);
}

QString FMCOMMS5_API::isBbDcOffsetTrackingEnabled()
{
	return readFromWidget("ad9361-phy/voltage0_in/bb_dc_offset_tracking_en");
}

void FMCOMMS5_API::setBbDcOffsetTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_in/bb_dc_offset_tracking_en", value);
}

// --- TX chain settings ---

QString FMCOMMS5_API::getTxRfBandwidth() { return readFromWidget("ad9361-phy/voltage0_out/rf_bandwidth"); }

void FMCOMMS5_API::setTxRfBandwidth(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_out/rf_bandwidth", value);
}

QString FMCOMMS5_API::getTxSamplingFrequency() { return readFromWidget("ad9361-phy/voltage0_out/sampling_frequency"); }

void FMCOMMS5_API::setTxSamplingFrequency(const QString &value)
{
	writeToWidget("ad9361-phy/voltage0_out/sampling_frequency", value);
}

QString FMCOMMS5_API::getTxRfPortSelect() { return readFromWidget("ad9361-phy/voltage0_out/rf_port_select"); }

void FMCOMMS5_API::setTxRfPortSelect(const QString &port)
{
	writeToWidget("ad9361-phy/voltage0_out/rf_port_select", port);
}

// --- Per-channel RX access ---

QString FMCOMMS5_API::getRxHardwareGain(int channel) { return readFromWidget(rxChannelKey(channel, "hardwaregain")); }

void FMCOMMS5_API::setRxHardwareGain(int channel, const QString &value)
{
	writeToWidget(rxChannelKey(channel, "hardwaregain"), value);
}

QString FMCOMMS5_API::getRxGainControlMode(int channel)
{
	return readFromWidget(rxChannelKey(channel, "gain_control_mode"));
}

void FMCOMMS5_API::setRxGainControlMode(int channel, const QString &mode)
{
	writeToWidget(rxChannelKey(channel, "gain_control_mode"), mode);
}

QString FMCOMMS5_API::getRxRssi(int channel) { return readFromWidget(rxChannelKey(channel, "rssi")); }

// --- Per-channel TX access ---

QString FMCOMMS5_API::getTxHardwareGain(int channel) { return readFromWidget(txChannelKey(channel, "hardwaregain")); }

void FMCOMMS5_API::setTxHardwareGain(int channel, const QString &value)
{
	writeToWidget(txChannelKey(channel, "hardwaregain"), value);
}

QString FMCOMMS5_API::getTxRssi(int channel) { return readFromWidget(txChannelKey(channel, "rssi")); }

// --- LO frequencies ---

QString FMCOMMS5_API::getRxLoFrequency(int device)
{
	return readFromWidget(deviceName(device) + "/altvoltage0_out/frequency");
}

void FMCOMMS5_API::setRxLoFrequency(int device, const QString &value)
{
	writeToWidget(deviceName(device) + "/altvoltage0_out/frequency", value);
}

QString FMCOMMS5_API::getTxLoFrequency(int device)
{
	return readFromWidget(deviceName(device) + "/altvoltage1_out/frequency");
}

void FMCOMMS5_API::setTxLoFrequency(int device, const QString &value)
{
	writeToWidget(deviceName(device) + "/altvoltage1_out/frequency", value);
}

// --- Generic widget access ---

QStringList FMCOMMS5_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString FMCOMMS5_API::readWidget(const QString &key) { return readFromWidget(key); }

void FMCOMMS5_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void FMCOMMS5_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_fmcomms5_api.cpp"
