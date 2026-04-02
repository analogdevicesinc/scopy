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

#ifndef AD9371_API_H
#define AD9371_API_H

#include "scopy-ad9371plugin_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad9371 {

class Ad9371Plugin;

class SCOPY_AD9371PLUGIN_EXPORT Ad9371_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Ad9371_API(Ad9371Plugin *plugin);
	~Ad9371_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// --- Global Settings (device attrs on ad9371-phy) ---
	Q_INVOKABLE QString getEnsmMode();
	Q_INVOKABLE void setEnsmMode(const QString &mode);
	Q_INVOKABLE QString getCalibrateRxQecEn();
	Q_INVOKABLE void setCalibrateRxQecEn(const QString &val);
	Q_INVOKABLE QString getCalibrateTxQecEn();
	Q_INVOKABLE void setCalibrateTxQecEn(const QString &val);
	Q_INVOKABLE QString getCalibrateTxLolEn();
	Q_INVOKABLE void setCalibrateTxLolEn(const QString &val);
	Q_INVOKABLE QString getCalibrateTxLolExtEn();
	Q_INVOKABLE void setCalibrateTxLolExtEn(const QString &val);
	Q_INVOKABLE QString getCalibrateDpdEn();
	Q_INVOKABLE void setCalibrateDpdEn(const QString &val);
	Q_INVOKABLE QString getCalibrateClgcEn();
	Q_INVOKABLE void setCalibrateClgcEn(const QString &val);
	Q_INVOKABLE QString getCalibrateVswrEn();
	Q_INVOKABLE void setCalibrateVswrEn(const QString &val);
	Q_INVOKABLE void calibrate();

	// --- RX Chain ---
	Q_INVOKABLE QString getRxRfBandwidth();
	Q_INVOKABLE QString getRxSamplingRate();
	Q_INVOKABLE QString getRxGainControlMode();
	Q_INVOKABLE void setRxGainControlMode(const QString &mode);
	Q_INVOKABLE QString getRxTempCompGain(int channel);
	Q_INVOKABLE void setRxTempCompGain(int channel, const QString &val);
	Q_INVOKABLE QString getRxHardwareGain(int channel);
	Q_INVOKABLE void setRxHardwareGain(int channel, const QString &val);
	Q_INVOKABLE QString getRxQuadratureTracking(int channel);
	Q_INVOKABLE void setRxQuadratureTracking(int channel, const QString &val);
	Q_INVOKABLE QString getRxRssi(int channel);
	Q_INVOKABLE QString getRxSamplingFrequency();
	Q_INVOKABLE void setRxSamplingFrequency(const QString &val);
	Q_INVOKABLE QString getRxLoFrequency();
	Q_INVOKABLE void setRxLoFrequency(const QString &val);

	// --- TX Chain ---
	Q_INVOKABLE QString getTxRfBandwidth();
	Q_INVOKABLE QString getTxSamplingRate();
	Q_INVOKABLE QString getTxAttenuation(int channel);
	Q_INVOKABLE void setTxAttenuation(int channel, const QString &val);
	Q_INVOKABLE QString getTxQuadratureTracking(int channel);
	Q_INVOKABLE void setTxQuadratureTracking(int channel, const QString &val);
	Q_INVOKABLE QString getTxLoLeakageTracking(int channel);
	Q_INVOKABLE void setTxLoLeakageTracking(int channel, const QString &val);
	Q_INVOKABLE QString getTxSamplingFrequency();
	Q_INVOKABLE void setTxSamplingFrequency(const QString &val);
	Q_INVOKABLE QString getTxLoFrequency();
	Q_INVOKABLE void setTxLoFrequency(const QString &val);

	// --- DPD Settings (per TX channel, conditional) ---
	Q_INVOKABLE QString getDpdTrackingEn(int channel);
	Q_INVOKABLE void setDpdTrackingEn(int channel, const QString &val);
	Q_INVOKABLE QString getDpdActuatorEn(int channel);
	Q_INVOKABLE void setDpdActuatorEn(int channel, const QString &val);
	Q_INVOKABLE void dpdReset(int channel);
	Q_INVOKABLE QString getDpdTrackCount(int channel);
	Q_INVOKABLE QString getDpdModelError(int channel);
	Q_INVOKABLE QString getDpdExternalPathDelay(int channel);
	Q_INVOKABLE QString getDpdStatus(int channel);

	// --- CLGC Settings (per TX channel, conditional) ---
	Q_INVOKABLE QString getClgcTrackingEn(int channel);
	Q_INVOKABLE void setClgcTrackingEn(int channel, const QString &val);
	Q_INVOKABLE QString getClgcDesiredGain(int channel);
	// clgc_desired_gain is read-only on the channel attribute; use debug attr adi,clgc-tx{1,2}-desired-gain for
	// writes
	Q_INVOKABLE QString getClgcTrackCount(int channel);
	Q_INVOKABLE QString getClgcStatus(int channel);
	Q_INVOKABLE QString getClgcCurrentGain(int channel);
	Q_INVOKABLE QString getClgcOrxRms(int channel);
	Q_INVOKABLE QString getClgcTxGain(int channel);
	Q_INVOKABLE QString getClgcTxRms(int channel);

	// --- VSWR Settings (per TX channel, conditional) ---
	Q_INVOKABLE QString getVswrTrackingEn(int channel);
	Q_INVOKABLE void setVswrTrackingEn(int channel, const QString &val);
	Q_INVOKABLE QString getVswrTrackCount(int channel);
	Q_INVOKABLE QString getVswrStatus(int channel);
	Q_INVOKABLE QString getVswrForwardGain(int channel);
	Q_INVOKABLE QString getVswrForwardGainImag(int channel);
	Q_INVOKABLE QString getVswrForwardGainReal(int channel);
	Q_INVOKABLE QString getVswrForwardOrx(int channel);
	Q_INVOKABLE QString getVswrForwardTx(int channel);
	Q_INVOKABLE QString getVswrReflectedGain(int channel);
	Q_INVOKABLE QString getVswrReflectedGainImag(int channel);
	Q_INVOKABLE QString getVswrReflectedGainReal(int channel);
	Q_INVOKABLE QString getVswrReflectedOrx(int channel);
	Q_INVOKABLE QString getVswrReflectedTx(int channel);

	// --- Observation/Sniffer RX ---
	Q_INVOKABLE QString getObsRfBandwidth();
	Q_INVOKABLE QString getObsSamplingRate();
	Q_INVOKABLE QString getObsGainControlMode();
	Q_INVOKABLE void setObsGainControlMode(const QString &mode);
	Q_INVOKABLE QString getObsRfPortSelect();
	Q_INVOKABLE void setObsRfPortSelect(const QString &port);
	Q_INVOKABLE QString getObsTempCompGain();
	Q_INVOKABLE void setObsTempCompGain(const QString &val);
	Q_INVOKABLE QString getObsHardwareGain();
	Q_INVOKABLE void setObsHardwareGain(const QString &val);
	Q_INVOKABLE QString getObsQuadratureTracking();
	Q_INVOKABLE void setObsQuadratureTracking(const QString &val);
	Q_INVOKABLE QString getObsRssi();
	Q_INVOKABLE QString getSnifferLoFrequency();
	Q_INVOKABLE void setSnifferLoFrequency(const QString &val);

	// --- Advanced Tool Navigation ---
	Q_INVOKABLE QStringList getAdvancedTabs();
	Q_INVOKABLE void switchAdvancedTab(const QString &name);

	// --- Generic Widget Access ---
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// --- Profile Loading ---
	Q_INVOKABLE void loadProfile(const QString &filePath);
	Q_INVOKABLE QString getDefaultProfilePath();

	// --- Phase Rotation (FPGA) ---
	Q_INVOKABLE QString getPhaseRotation(int channel);
	Q_INVOKABLE void setPhaseRotation(int channel, double degrees);

	// --- Up/Down Converter ---
	Q_INVOKABLE bool getUdcEnabled();
	Q_INVOKABLE void setUdcEnabled(bool enabled);
	Q_INVOKABLE bool hasUdc();

	// --- FPGA Frequency ---
	Q_INVOKABLE QString getFpgaTxFrequency();
	Q_INVOKABLE void setFpgaTxFrequency(const QString &value);
	Q_INVOKABLE QString getFpgaRxFrequency();
	Q_INVOKABLE void setFpgaRxFrequency(const QString &value);

	// --- Utility ---
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);
	void writeRawToWidget(const QString &key, const QString &value);
	QString txChannelKey(int channel, const QString &attr);
	static QString stripUnitSuffix(const QString &value);

	Ad9371Plugin *m_plugin;
};

} // namespace scopy::ad9371

#endif // AD9371_API_H
