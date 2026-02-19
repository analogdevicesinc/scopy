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

#include "adrv9002_api.h"
#include "adrv9002plugin.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ADRV9002_API, "ADRV9002_API")

using namespace scopy::adrv9002;

ADRV9002_API::ADRV9002_API(Adrv9002Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

ADRV9002_API::~ADRV9002_API() {}

// --- Key helpers ---

QString ADRV9002_API::rxKey(int ch, const QString &attr)
{
	return m_plugin->m_devName + QString("/voltage%1_in/").arg(ch) + attr;
}

QString ADRV9002_API::txKey(int ch, const QString &attr)
{
	return m_plugin->m_devName + QString("/voltage%1_out/").arg(ch) + attr;
}

QString ADRV9002_API::rxLoKey(int ch)
{
	return m_plugin->m_devName + QString("/altvoltage%1_out/RX%2_LO_frequency").arg(ch).arg(ch + 1);
}

QString ADRV9002_API::txLoKey(int ch)
{
	return m_plugin->m_devName + QString("/altvoltage%1_out/TX%2_LO_frequency").arg(ch + 2).arg(ch + 1);
}

QString ADRV9002_API::orxKey(int ch, const QString &attr)
{
	return m_plugin->m_devName + QString("/voltage%1_in/").arg(ch) + attr;
}

// --- Private helpers ---

QString ADRV9002_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_ADRV9002_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void ADRV9002_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_ADRV9002_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList ADRV9002_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Global ---

QString ADRV9002_API::getTemperature() { return readFromWidget(m_plugin->m_devName + "/temp0_in/input"); }

// --- RX ---

QString ADRV9002_API::getRxHardwareGain(int ch) { return readFromWidget(rxKey(ch, "hardwaregain")); }
void ADRV9002_API::setRxHardwareGain(int ch, const QString &val) { writeToWidget(rxKey(ch, "hardwaregain"), val); }

QString ADRV9002_API::getRxGainControlMode(int ch) { return readFromWidget(rxKey(ch, "gain_control_mode")); }
void ADRV9002_API::setRxGainControlMode(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "gain_control_mode"), val);
}

QString ADRV9002_API::getRxEnsmMode(int ch) { return readFromWidget(rxKey(ch, "ensm_mode")); }
void ADRV9002_API::setRxEnsmMode(int ch, const QString &val) { writeToWidget(rxKey(ch, "ensm_mode"), val); }

QString ADRV9002_API::isRxEnabled(int ch) { return readFromWidget(rxKey(ch, "en")); }
void ADRV9002_API::setRxEnabled(int ch, const QString &val) { writeToWidget(rxKey(ch, "en"), val); }

QString ADRV9002_API::isRxBbdcRejectionEnabled(int ch) { return readFromWidget(rxKey(ch, "bbdc_rejection_en")); }
void ADRV9002_API::setRxBbdcRejectionEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "bbdc_rejection_en"), val);
}

QString ADRV9002_API::getRxNcoFrequency(int ch) { return readFromWidget(rxKey(ch, "nco_frequency")); }
void ADRV9002_API::setRxNcoFrequency(int ch, const QString &val) { writeToWidget(rxKey(ch, "nco_frequency"), val); }

QString ADRV9002_API::getRxDecimatedPower(int ch) { return readFromWidget(rxKey(ch, "decimated_power")); }

QString ADRV9002_API::getRxRfBandwidth(int ch) { return readFromWidget(rxKey(ch, "rf_bandwidth")); }

QString ADRV9002_API::getRxDigitalGainControlMode(int ch)
{
	return readFromWidget(rxKey(ch, "digital_gain_control_mode"));
}
void ADRV9002_API::setRxDigitalGainControlMode(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "digital_gain_control_mode"), val);
}

QString ADRV9002_API::getRxInterfaceGain(int ch) { return readFromWidget(rxKey(ch, "interface_gain")); }
void ADRV9002_API::setRxInterfaceGain(int ch, const QString &val) { writeToWidget(rxKey(ch, "interface_gain"), val); }

QString ADRV9002_API::getRxPortEnMode(int ch) { return readFromWidget(rxKey(ch, "port_en_mode")); }
void ADRV9002_API::setRxPortEnMode(int ch, const QString &val) { writeToWidget(rxKey(ch, "port_en_mode"), val); }

QString ADRV9002_API::isRxDynamicAdcSwitchEnabled(int ch) { return readFromWidget(rxKey(ch, "dynamic_adc_switch_en")); }
void ADRV9002_API::setRxDynamicAdcSwitchEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "dynamic_adc_switch_en"), val);
}

QString ADRV9002_API::getRxBbdcLoopGain(int ch) { return readFromWidget(rxKey(ch, "bbdc_loop_gain_raw")); }
void ADRV9002_API::setRxBbdcLoopGain(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "bbdc_loop_gain_raw"), val);
}

QString ADRV9002_API::getRxLoFrequency(int ch) { return readFromWidget(rxLoKey(ch)); }
void ADRV9002_API::setRxLoFrequency(int ch, const QString &val) { writeToWidget(rxLoKey(ch), val); }

QString ADRV9002_API::getRxRssi(int ch) { return readFromWidget(rxKey(ch, "rssi")); }

QString ADRV9002_API::getRxSamplingFrequency(int ch) { return readFromWidget(rxKey(ch, "sampling_frequency")); }

// --- RX Tracking ---

QString ADRV9002_API::isRxQuadratureFicTrackingEnabled(int ch)
{
	return readFromWidget(rxKey(ch, "quadrature_fic_tracking_en"));
}
void ADRV9002_API::setRxQuadratureFicTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "quadrature_fic_tracking_en"), val);
}

QString ADRV9002_API::isRxAgcTrackingEnabled(int ch) { return readFromWidget(rxKey(ch, "agc_tracking_en")); }
void ADRV9002_API::setRxAgcTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "agc_tracking_en"), val);
}

QString ADRV9002_API::isRxBbdcRejectionTrackingEnabled(int ch)
{
	return readFromWidget(rxKey(ch, "bbdc_rejection_tracking_en"));
}
void ADRV9002_API::setRxBbdcRejectionTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "bbdc_rejection_tracking_en"), val);
}

QString ADRV9002_API::isRxQuadraturePolyTrackingEnabled(int ch)
{
	return readFromWidget(rxKey(ch, "quadrature_w_poly_tracking_en"));
}
void ADRV9002_API::setRxQuadraturePolyTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "quadrature_w_poly_tracking_en"), val);
}

QString ADRV9002_API::isRxHdTrackingEnabled(int ch) { return readFromWidget(rxKey(ch, "hd_tracking_en")); }
void ADRV9002_API::setRxHdTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "hd_tracking_en"), val);
}

QString ADRV9002_API::isRxRssiTrackingEnabled(int ch) { return readFromWidget(rxKey(ch, "rssi_tracking_en")); }
void ADRV9002_API::setRxRssiTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(rxKey(ch, "rssi_tracking_en"), val);
}

// --- TX ---

QString ADRV9002_API::getTxAttenuation(int ch) { return readFromWidget(txKey(ch, "hardwaregain")); }
void ADRV9002_API::setTxAttenuation(int ch, const QString &val) { writeToWidget(txKey(ch, "hardwaregain"), val); }

QString ADRV9002_API::getTxAttenControlMode(int ch) { return readFromWidget(txKey(ch, "atten_control_mode")); }
void ADRV9002_API::setTxAttenControlMode(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "atten_control_mode"), val);
}

QString ADRV9002_API::getTxLoFrequency(int ch) { return readFromWidget(txLoKey(ch)); }
void ADRV9002_API::setTxLoFrequency(int ch, const QString &val) { writeToWidget(txLoKey(ch), val); }

QString ADRV9002_API::getTxNcoFrequency(int ch) { return readFromWidget(txKey(ch, "nco_frequency")); }
void ADRV9002_API::setTxNcoFrequency(int ch, const QString &val) { writeToWidget(txKey(ch, "nco_frequency"), val); }

QString ADRV9002_API::getTxRfBandwidth(int ch) { return readFromWidget(txKey(ch, "rf_bandwidth")); }

QString ADRV9002_API::getTxPortEnMode(int ch) { return readFromWidget(txKey(ch, "port_en_mode")); }
void ADRV9002_API::setTxPortEnMode(int ch, const QString &val) { writeToWidget(txKey(ch, "port_en_mode"), val); }

QString ADRV9002_API::getTxEnsmMode(int ch) { return readFromWidget(txKey(ch, "ensm_mode")); }
void ADRV9002_API::setTxEnsmMode(int ch, const QString &val) { writeToWidget(txKey(ch, "ensm_mode"), val); }

QString ADRV9002_API::isTxEnabled(int ch) { return readFromWidget(txKey(ch, "en")); }
void ADRV9002_API::setTxEnabled(int ch, const QString &val) { writeToWidget(txKey(ch, "en"), val); }

QString ADRV9002_API::getTxSamplingFrequency(int ch) { return readFromWidget(txKey(ch, "sampling_frequency")); }

// --- TX Tracking ---

QString ADRV9002_API::isTxQuadratureTrackingEnabled(int ch)
{
	return readFromWidget(txKey(ch, "quadrature_tracking_en"));
}
void ADRV9002_API::setTxQuadratureTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "quadrature_tracking_en"), val);
}

QString ADRV9002_API::isTxCloseLoopGainTrackingEnabled(int ch)
{
	return readFromWidget(txKey(ch, "close_loop_gain_tracking_en"));
}
void ADRV9002_API::setTxCloseLoopGainTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "close_loop_gain_tracking_en"), val);
}

QString ADRV9002_API::isTxPaCorrectionTrackingEnabled(int ch)
{
	return readFromWidget(txKey(ch, "pa_correction_tracking_en"));
}
void ADRV9002_API::setTxPaCorrectionTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "pa_correction_tracking_en"), val);
}

QString ADRV9002_API::isTxLoopbackDelayTrackingEnabled(int ch)
{
	return readFromWidget(txKey(ch, "loopback_delay_tracking_en"));
}
void ADRV9002_API::setTxLoopbackDelayTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "loopback_delay_tracking_en"), val);
}

QString ADRV9002_API::isTxLoLeakageTrackingEnabled(int ch)
{
	return readFromWidget(txKey(ch, "lo_leakage_tracking_en"));
}
void ADRV9002_API::setTxLoLeakageTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(txKey(ch, "lo_leakage_tracking_en"), val);
}

// --- ORX ---

QString ADRV9002_API::getOrxHardwareGain(int ch) { return readFromWidget(orxKey(ch, "orx_hardwaregain")); }
void ADRV9002_API::setOrxHardwareGain(int ch, const QString &val)
{
	writeToWidget(orxKey(ch, "orx_hardwaregain"), val);
}

QString ADRV9002_API::isOrxBbdcRejectionEnabled(int ch) { return readFromWidget(orxKey(ch, "orx_bbdc_rejection_en")); }
void ADRV9002_API::setOrxBbdcRejectionEnabled(int ch, const QString &val)
{
	writeToWidget(orxKey(ch, "orx_bbdc_rejection_en"), val);
}

QString ADRV9002_API::isOrxQuadraturePolyTrackingEnabled(int ch)
{
	return readFromWidget(orxKey(ch, "orx_quadrature_w_poly_tracking_en"));
}
void ADRV9002_API::setOrxQuadraturePolyTrackingEnabled(int ch, const QString &val)
{
	writeToWidget(orxKey(ch, "orx_quadrature_w_poly_tracking_en"), val);
}

QString ADRV9002_API::isOrxEnabled(int ch) { return readFromWidget(orxKey(ch, "orx_en")); }
void ADRV9002_API::setOrxEnabled(int ch, const QString &val) { writeToWidget(orxKey(ch, "orx_en"), val); }

// --- Generic widget access ---

QStringList ADRV9002_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString ADRV9002_API::readWidget(const QString &key) { return readFromWidget(key); }

void ADRV9002_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Utility ---

void ADRV9002_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_adrv9002_api.cpp"
