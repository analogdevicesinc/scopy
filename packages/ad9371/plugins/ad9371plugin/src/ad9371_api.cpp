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

#include "ad9371_api.h"
#include "ad9371plugin.h"
#include "ad9371.h"
#include "ad9371advanced.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <iioutil/connectionprovider.h>
#include <QLoggingCategory>
#include <iio.h>
#include <cmath>
#include <QDir>
#include <pkg-manager/pkgmanager.h>

Q_LOGGING_CATEGORY(CAT_AD9371_API, "AD9371_API")

using namespace scopy::ad9371;

static const char *dpd_status_strings[] = {
	"No Error",
	"Error: ORx disabled",
	"Error: Tx disabled",
	"Error: DPD initialization not run",
	"Error: Path delay not setup",
	"Error: ORx signal too low",
	"Error: ORx signal saturated",
	"Error: Tx signal too low",
	"Error: Tx signal saturated",
	"Error: Model error high",
	"Error: AM AM outliers",
	"Error: Invalid Tx profile",
	"Error: ORx QEC Disabled",
};

static const char *clgc_status_strings[] = {
	"No Error",
	"Error: TX is disabled",
	"Error: ORx is disabled",
	"Error: Loopback switch is closed",
	"Error: Data measurement aborted during capture",
	"Error: No initial calibration was done",
	"Error: Path delay not setup",
	"Error: No apply control is possible",
	"Error: Control value is out of range",
	"Error: CLGC feature is disabled",
	"Error: TX attenuation is capped",
	"Error: Gain measurement",
	"Error: No GPIO configured in single ORx configuration",
	"Error: Tx is not observable with any of the ORx Channels",
};

static const char *vswr_status_strings[] = {
	"No Error",
	"Error: TX disabled",
	"Error: ORx disabled",
	"Error: Loopback switch is closed",
	"Error: No initial calibration was done",
	"Error: Path delay not setup",
	"Error: Data capture aborted",
	"Error: VSWR is disabled",
	"Error: Entering Cal",
	"Error: No GPIO configured in single ORx configuration",
	"Error: Tx is not observable with any of the ORx Channels",
};

Ad9371_API::Ad9371_API(Ad9371Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Ad9371_API::~Ad9371_API() {}

// --- Private helpers ---

QString Ad9371_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD9371_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD9371_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return stripUnitSuffix(result.first);
}

QString Ad9371_API::stripUnitSuffix(const QString &value)
{
	if(value.isEmpty())
		return value;

	// If the value parses as a number directly, return as-is
	bool ok;
	value.toDouble(&ok);
	if(ok)
		return value;

	// Find the boundary between numeric part and unit suffix (e.g. "-97.500000 dB")
	QString trimmed = value.trimmed();
	int unitStart = -1;
	for(int i = 0; i < trimmed.length(); ++i) {
		QChar ch = trimmed.at(i);
		if(ch.isSpace() && i > 0) {
			unitStart = i;
			break;
		}
		if(ch.isLetter() && i > 0) {
			unitStart = i;
			break;
		}
	}

	if(unitStart > 0) {
		QString numberPart = trimmed.left(unitStart).trimmed();
		numberPart.toDouble(&ok);
		if(ok)
			return numberPart;
	}

	return value;
}

void Ad9371_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD9371_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD9371_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

QString Ad9371_API::txChannelKey(int channel, const QString &attr)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return "ad9371-phy/" + chn + "_out/" + attr;
}

// --- Tool management ---

QStringList Ad9371_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Global Settings ---

QString Ad9371_API::getEnsmMode() { return readFromWidget("ad9371-phy/ensm_mode"); }

void Ad9371_API::setEnsmMode(const QString &mode) { writeToWidget("ad9371-phy/ensm_mode", mode); }

QString Ad9371_API::getCalibrateRxQecEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_rx_qec_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateRxQecEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_rx_qec_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateTxQecEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_tx_qec_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateTxQecEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_tx_qec_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateTxLolEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_tx_lol_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateTxLolEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_tx_lol_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateTxLolExtEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_tx_lol_ext_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateTxLolExtEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_tx_lol_ext_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateDpdEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_dpd_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateDpdEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_dpd_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateClgcEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_clgc_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateClgcEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_clgc_en", v.toInt() != 0);
}

QString Ad9371_API::getCalibrateVswrEn()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return QString();
	bool val;
	int ret = iio_device_attr_read_bool(tool->m_dev, "calibrate_vswr_en", &val);
	return (ret >= 0) ? QString::number(val ? 1 : 0) : QString();
}

void Ad9371_API::setCalibrateVswrEn(const QString &v)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_dev)
		return;
	iio_device_attr_write_bool(tool->m_dev, "calibrate_vswr_en", v.toInt() != 0);
}

void Ad9371_API::calibrate()
{
	// Direct IIO write — no widget for write-only trigger
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_plugin->m_param);
	if(!conn) {
		qWarning(CAT_AD9371_API) << "Failed to open connection for calibrate";
		return;
	}
	iio_device *dev = iio_context_find_device(conn->context(), "ad9371-phy");
	if(!dev) {
		qWarning(CAT_AD9371_API) << "ad9371-phy device not found";
		return;
	}
	int ret = iio_device_attr_write_bool(dev, "calibrate", true);
	if(ret < 0)
		qWarning(CAT_AD9371_API) << "Calibration failed:" << ret;
	else
		qDebug(CAT_AD9371_API) << "Calibration triggered";
}

// --- RX Chain ---

QString Ad9371_API::getRxRfBandwidth()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_in/rf_bandwidth");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getRxSamplingRate()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_in/sampling_frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getRxGainControlMode() { return readFromWidget("ad9371-phy/voltage0_in/gain_control_mode"); }

void Ad9371_API::setRxGainControlMode(const QString &mode)
{
	writeToWidget("ad9371-phy/voltage0_in/gain_control_mode", mode);
}

QString Ad9371_API::getRxTempCompGain(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9371-phy/" + chn + "_in/temp_comp_gain");
}

void Ad9371_API::setRxTempCompGain(int channel, const QString &val)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9371-phy/" + chn + "_in/temp_comp_gain", val);
}

QString Ad9371_API::getRxHardwareGain(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	QString raw = readFromWidget("ad9371-phy/" + chn + "_in/hardwaregain");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble(), 'f', 2);
}

void Ad9371_API::setRxHardwareGain(int channel, const QString &val)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9371-phy/" + chn + "_in/hardwaregain", QString::number(val.toDouble(), 'f', 2));
}

QString Ad9371_API::getRxQuadratureTracking(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9371-phy/" + chn + "_in/quadrature_tracking_en");
}

void Ad9371_API::setRxQuadratureTracking(int channel, const QString &val)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	writeToWidget("ad9371-phy/" + chn + "_in/quadrature_tracking_en", val);
}

QString Ad9371_API::getRxRssi(int channel)
{
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	return readFromWidget("ad9371-phy/" + chn + "_in/rssi");
}

QString Ad9371_API::getRxSamplingFrequency()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_in/sampling_frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Ad9371_API::setRxSamplingFrequency(const QString &val)
{
	writeToWidget("ad9371-phy/voltage0_in/sampling_frequency", QString::number(val.toDouble() * 1e6, 'f', 0));
}

QString Ad9371_API::getRxLoFrequency()
{
	QString raw = readFromWidget("ad9371-phy/altvoltage0_out/frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Ad9371_API::setRxLoFrequency(const QString &val)
{
	writeToWidget("ad9371-phy/altvoltage0_out/frequency", QString::number(val.toDouble() * 1e6, 'f', 0));
}

// --- TX Chain ---

QString Ad9371_API::getTxRfBandwidth()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_out/rf_bandwidth");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getTxSamplingRate()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_out/sampling_frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getTxAttenuation(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "hardwaregain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(-raw.toDouble(), 'f', 2);
}

void Ad9371_API::setTxAttenuation(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "hardwaregain"), QString::number(-std::abs(val.toDouble()), 'f', 2));
}

QString Ad9371_API::getTxQuadratureTracking(int channel)
{
	return readFromWidget(txChannelKey(channel, "quadrature_tracking_en"));
}

void Ad9371_API::setTxQuadratureTracking(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "quadrature_tracking_en"), val);
}

QString Ad9371_API::getTxLoLeakageTracking(int channel)
{
	return readFromWidget(txChannelKey(channel, "lo_leakage_tracking_en"));
}

void Ad9371_API::setTxLoLeakageTracking(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "lo_leakage_tracking_en"), val);
}

QString Ad9371_API::getTxSamplingFrequency()
{
	QString raw = readFromWidget("ad9371-phy/voltage0_out/sampling_frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Ad9371_API::setTxSamplingFrequency(const QString &val)
{
	writeToWidget("ad9371-phy/voltage0_out/sampling_frequency", QString::number(val.toDouble() * 1e6, 'f', 0));
}

QString Ad9371_API::getTxLoFrequency()
{
	QString raw = readFromWidget("ad9371-phy/altvoltage1_out/frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Ad9371_API::setTxLoFrequency(const QString &val)
{
	writeToWidget("ad9371-phy/altvoltage1_out/frequency", QString::number(val.toDouble() * 1e6, 'f', 0));
}

// --- DPD Settings ---

QString Ad9371_API::getDpdTrackingEn(int channel) { return readFromWidget(txChannelKey(channel, "dpd_tracking_en")); }

void Ad9371_API::setDpdTrackingEn(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "dpd_tracking_en"), val);
}

QString Ad9371_API::getDpdActuatorEn(int channel) { return readFromWidget(txChannelKey(channel, "dpd_actuator_en")); }

void Ad9371_API::setDpdActuatorEn(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "dpd_actuator_en"), val);
}

void Ad9371_API::dpdReset(int channel)
{
	// Direct IIO write — dpd_reset_en is a write-only trigger
	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_plugin->m_param);
	if(!conn) {
		qWarning(CAT_AD9371_API) << "Failed to open connection for DPD reset";
		return;
	}
	iio_device *dev = iio_context_find_device(conn->context(), "ad9371-phy");
	if(!dev) {
		qWarning(CAT_AD9371_API) << "ad9371-phy device not found";
		return;
	}
	QString chn = (channel == 0) ? "voltage0" : "voltage1";
	iio_channel *ch = iio_device_find_channel(dev, chn.toLatin1(), true);
	if(ch) {
		iio_channel_attr_write_bool(ch, "dpd_reset_en", true);
	}
}

QString Ad9371_API::getDpdTrackCount(int channel) { return readFromWidget(txChannelKey(channel, "dpd_track_count")); }

QString Ad9371_API::getDpdModelError(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "dpd_model_error"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 10.0, 'f', 1) + " %";
}

QString Ad9371_API::getDpdExternalPathDelay(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "dpd_external_path_delay"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 16.0, 'f', 2);
}

QString Ad9371_API::getDpdStatus(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "dpd_status"));
	if(raw.isEmpty())
		return raw;
	int idx = raw.toInt();
	int count = (int)(sizeof(dpd_status_strings) / sizeof(dpd_status_strings[0]));
	if(idx >= 0 && idx < count)
		return QString(dpd_status_strings[idx]);
	return QString("Error: Unknown status (%1)").arg(idx);
}

// --- CLGC Settings ---

QString Ad9371_API::getClgcTrackingEn(int channel) { return readFromWidget(txChannelKey(channel, "clgc_tracking_en")); }

void Ad9371_API::setClgcTrackingEn(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "clgc_tracking_en"), val);
}

QString Ad9371_API::getClgcDesiredGain(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_desired_gain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 100.0, 'f', 2);
}

QString Ad9371_API::getClgcTrackCount(int channel) { return readFromWidget(txChannelKey(channel, "clgc_track_count")); }

QString Ad9371_API::getClgcStatus(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_status"));
	if(raw.isEmpty())
		return raw;
	int idx = raw.toInt();
	int count = (int)(sizeof(clgc_status_strings) / sizeof(clgc_status_strings[0]));
	if(idx >= 0 && idx < count)
		return QString(clgc_status_strings[idx]);
	return QString("Error: Unknown status (%1)").arg(idx);
}

QString Ad9371_API::getClgcCurrentGain(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_current_gain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getClgcOrxRms(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_orx_rms"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dBFS";
}

QString Ad9371_API::getClgcTxGain(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_tx_gain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 20.0, 'f', 2) + " dB";
}

QString Ad9371_API::getClgcTxRms(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "clgc_tx_rms"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dBFS";
}

// --- VSWR Settings ---

QString Ad9371_API::getVswrTrackingEn(int channel) { return readFromWidget(txChannelKey(channel, "vswr_tracking_en")); }

void Ad9371_API::setVswrTrackingEn(int channel, const QString &val)
{
	writeToWidget(txChannelKey(channel, "vswr_tracking_en"), val);
}

QString Ad9371_API::getVswrTrackCount(int channel) { return readFromWidget(txChannelKey(channel, "vswr_track_count")); }

QString Ad9371_API::getVswrStatus(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_status"));
	if(raw.isEmpty())
		return raw;
	int idx = raw.toInt();
	int count = (int)(sizeof(vswr_status_strings) / sizeof(vswr_status_strings[0]));
	if(idx >= 0 && idx < count)
		return QString(vswr_status_strings[idx]);
	return QString("Error: Unknown status (%1)").arg(idx);
}

QString Ad9371_API::getVswrForwardGain(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_forward_gain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrForwardGainImag(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_forward_gain_imag"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrForwardGainReal(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_forward_gain_real"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrForwardOrx(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_forward_orx"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0 + 21.0, 'f', 2) + " dBFS";
}

QString Ad9371_API::getVswrForwardTx(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_forward_tx"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0 + 21.0, 'f', 2) + " dBFS";
}

QString Ad9371_API::getVswrReflectedGain(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_reflected_gain"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrReflectedGainImag(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_reflected_gain_imag"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrReflectedGainReal(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_reflected_gain_real"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0, 'f', 2) + " dB";
}

QString Ad9371_API::getVswrReflectedOrx(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_reflected_orx"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0 + 21.0, 'f', 2) + " dBFS";
}

QString Ad9371_API::getVswrReflectedTx(int channel)
{
	QString raw = readFromWidget(txChannelKey(channel, "vswr_reflected_tx"));
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toLongLong() / 100.0 + 21.0, 'f', 2) + " dBFS";
}

// --- Observation/Sniffer RX ---

QString Ad9371_API::getObsRfBandwidth()
{
	QString raw = readFromWidget("ad9371-phy/voltage2_in/rf_bandwidth");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getObsSamplingRate()
{
	QString raw = readFromWidget("ad9371-phy/voltage2_in/sampling_frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

QString Ad9371_API::getObsGainControlMode() { return readFromWidget("ad9371-phy/voltage2_in/gain_control_mode"); }

void Ad9371_API::setObsGainControlMode(const QString &mode)
{
	writeToWidget("ad9371-phy/voltage2_in/gain_control_mode", mode);
}

QString Ad9371_API::getObsRfPortSelect() { return readFromWidget("ad9371-phy/voltage2_in/rf_port_select"); }

void Ad9371_API::setObsRfPortSelect(const QString &port)
{
	writeToWidget("ad9371-phy/voltage2_in/rf_port_select", port);
}

QString Ad9371_API::getObsTempCompGain() { return readFromWidget("ad9371-phy/voltage2_in/temp_comp_gain"); }

void Ad9371_API::setObsTempCompGain(const QString &val) { writeToWidget("ad9371-phy/voltage2_in/temp_comp_gain", val); }

QString Ad9371_API::getObsHardwareGain()
{
	QString raw = readFromWidget("ad9371-phy/voltage2_in/hardwaregain");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble(), 'f', 2);
}

void Ad9371_API::setObsHardwareGain(const QString &val)
{
	writeToWidget("ad9371-phy/voltage2_in/hardwaregain", QString::number(val.toDouble(), 'f', 2));
}

QString Ad9371_API::getObsQuadratureTracking()
{
	return readFromWidget("ad9371-phy/voltage2_in/quadrature_tracking_en");
}

void Ad9371_API::setObsQuadratureTracking(const QString &val)
{
	writeToWidget("ad9371-phy/voltage2_in/quadrature_tracking_en", val);
}

QString Ad9371_API::getObsRssi() { return readFromWidget("ad9371-phy/voltage2_in/rssi"); }

QString Ad9371_API::getSnifferLoFrequency()
{
	QString raw = readFromWidget("ad9371-phy/altvoltage2_out/frequency");
	if(raw.isEmpty())
		return raw;
	return QString::number(raw.toDouble() / 1e6, 'f', 6);
}

void Ad9371_API::setSnifferLoFrequency(const QString &val)
{
	writeToWidget("ad9371-phy/altvoltage2_out/frequency", QString::number(val.toDouble() * 1e6, 'f', 0));
}

// --- Advanced Tool Navigation ---

QStringList Ad9371_API::getAdvancedTabs()
{
	if(m_plugin->m_toolList.size() < 2)
		return QStringList();

	QWidget *tool = m_plugin->m_toolList[1]->tool();
	Ad9371Advanced *advanced = qobject_cast<Ad9371Advanced *>(tool);
	if(!advanced) {
		qWarning(CAT_AD9371_API) << "Advanced tool not available";
		return QStringList();
	}
	return advanced->getSections();
}

void Ad9371_API::switchAdvancedTab(const QString &name)
{
	if(m_plugin->m_toolList.size() < 2)
		return;

	QWidget *tool = m_plugin->m_toolList[1]->tool();
	Ad9371Advanced *advanced = qobject_cast<Ad9371Advanced *>(tool);
	if(!advanced) {
		qWarning(CAT_AD9371_API) << "Advanced tool not available";
		return;
	}
	advanced->switchToSection(name);
}

// --- Generic Widget Access ---

QStringList Ad9371_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString Ad9371_API::readWidget(const QString &key) { return readFromWidget(key); }

void Ad9371_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Profile Loading ---

void Ad9371_API::loadProfile(const QString &filePath)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool) {
		qWarning(CAT_AD9371_API) << "AD9371 tool not available";
		return;
	}
	tool->loadProfileFromFile(filePath);
}

QString Ad9371_API::getDefaultProfilePath()
{
	QFileInfoList filterFiles = PkgManager::listFilesInfo(QStringList() << "ad9371-filters");
	if(filterFiles.isEmpty())
		return QString();

	// Return the first .txt profile file found
	for(const QFileInfo &fi : filterFiles) {
		if(fi.suffix() == "txt")
			return fi.absoluteFilePath();
	}
	return filterFiles.first().absoluteFilePath();
}

// --- Phase Rotation ---

QString Ad9371_API::getPhaseRotation(int channel)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_cap)
		return QString();

	QString i_ch = QString("voltage%1_i").arg(channel);
	QString q_ch = QString("voltage%1_q").arg(channel);

	iio_channel *i_chn = iio_device_find_channel(tool->m_cap, i_ch.toLatin1(), false);
	iio_channel *q_chn = iio_device_find_channel(tool->m_cap, q_ch.toLatin1(), false);

	if(!i_chn || !q_chn)
		return QString();

	double val[4];
	if(iio_channel_attr_read_double(i_chn, "calibscale", &val[0]) != 0 ||
	   iio_channel_attr_read_double(i_chn, "calibphase", &val[1]) != 0 ||
	   iio_channel_attr_read_double(q_chn, "calibscale", &val[2]) != 0 ||
	   iio_channel_attr_read_double(q_chn, "calibphase", &val[3]) != 0)
		return QString();

	val[0] = acos(val[0]) * 360.0 / (2.0 * M_PI);
	val[1] = asin(-1.0 * val[1]) * 360.0 / (2.0 * M_PI);
	val[2] = acos(val[2]) * 360.0 / (2.0 * M_PI);
	val[3] = asin(val[3]) * 360.0 / (2.0 * M_PI);

	if(val[1] < 0.0)
		val[0] *= -1.0;
	if(val[3] < 0.0)
		val[2] *= -1.0;
	if(val[1] < -90.0)
		val[0] = (val[0] * -1.0) - 180.0;
	if(val[3] < -90.0)
		val[2] = (val[2] * -1.0) - 180.0;

	if(fabs(val[0]) > 90.0) {
		if(val[1] < 0.0)
			val[1] = (val[1] * -1.0) - 180.0;
		else
			val[1] = 180.0 - val[1];
	}
	if(fabs(val[2]) > 90.0) {
		if(val[3] < 0.0)
			val[3] = (val[3] * -1.0) - 180.0;
		else
			val[3] = 180.0 - val[3];
	}

	double degrees;
	if(round(val[0]) != round(val[1]) && round(val[0]) != round(val[2]) && round(val[0]) != round(val[3])) {
		degrees = 0.0;
	} else {
		degrees = (val[0] + val[1] + val[2] + val[3]) / 4.0;
	}

	return QString::number(degrees, 'f', 1);
}

void Ad9371_API::setPhaseRotation(int channel, double degrees)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_cap)
		return;

	double phase = degrees * 2.0 * M_PI / 360.0;

	QString i_ch = QString("voltage%1_i").arg(channel);
	QString q_ch = QString("voltage%1_q").arg(channel);

	iio_channel *i_chn = iio_device_find_channel(tool->m_cap, i_ch.toLatin1(), false);
	iio_channel *q_chn = iio_device_find_channel(tool->m_cap, q_ch.toLatin1(), false);

	if(i_chn && q_chn) {
		iio_channel_attr_write_double(i_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(i_chn, "calibphase", -sin(phase));
		iio_channel_attr_write_double(q_chn, "calibscale", cos(phase));
		iio_channel_attr_write_double(q_chn, "calibphase", sin(phase));
	}
}

// --- Up/Down Converter ---

bool Ad9371_API::hasUdc()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool)
		return false;
	return tool->m_hasUdc;
}

bool Ad9371_API::getUdcEnabled()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool)
		return false;
	return tool->m_udcEnabled;
}

void Ad9371_API::setUdcEnabled(bool enabled)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_hasUdc)
		return;
	tool->onUdcToggled(enabled);
}

// --- FPGA Frequency ---

QString Ad9371_API::getFpgaTxFrequency()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_fpgaTxFreqCombo)
		return QString();
	QPair<QString, QString> result = tool->m_fpgaTxFreqCombo->read();
	return result.first;
}

void Ad9371_API::setFpgaTxFrequency(const QString &value)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_fpgaTxFreqCombo)
		return;
	tool->m_fpgaTxFreqCombo->writeAsync(value);
}

QString Ad9371_API::getFpgaRxFrequency()
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_fpgaRxFreqCombo)
		return QString();
	QPair<QString, QString> result = tool->m_fpgaRxFreqCombo->read();
	return result.first;
}

void Ad9371_API::setFpgaRxFrequency(const QString &value)
{
	Ad9371 *tool = m_plugin->m_ad9371Tool;
	if(!tool || !tool->m_fpgaRxFreqCombo)
		return;
	tool->m_fpgaRxFreqCombo->writeAsync(value);
}

// --- Utility ---

void Ad9371_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_ad9371_api.cpp"
