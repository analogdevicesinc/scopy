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

#ifndef ADRV9002_API_H
#define ADRV9002_API_H

#include "scopy-adrv9002plugin_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::adrv9002 {

class Adrv9002Plugin;

class SCOPY_ADRV9002PLUGIN_EXPORT ADRV9002_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ADRV9002_API(Adrv9002Plugin *plugin);
	~ADRV9002_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Global
	Q_INVOKABLE QString getTemperature();

	// RX (per-channel, ch=0 or 1)
	Q_INVOKABLE QString getRxHardwareGain(int ch);
	Q_INVOKABLE void setRxHardwareGain(int ch, const QString &val);
	Q_INVOKABLE QString getRxGainControlMode(int ch);
	Q_INVOKABLE void setRxGainControlMode(int ch, const QString &val);
	Q_INVOKABLE QString getRxEnsmMode(int ch);
	Q_INVOKABLE void setRxEnsmMode(int ch, const QString &val);
	Q_INVOKABLE QString isRxEnabled(int ch);
	Q_INVOKABLE void setRxEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxBbdcRejectionEnabled(int ch);
	Q_INVOKABLE void setRxBbdcRejectionEnabled(int ch, const QString &val);
	Q_INVOKABLE QString getRxNcoFrequency(int ch);
	Q_INVOKABLE void setRxNcoFrequency(int ch, const QString &val);
	Q_INVOKABLE QString getRxDecimatedPower(int ch);
	Q_INVOKABLE QString getRxRfBandwidth(int ch);
	Q_INVOKABLE QString getRxDigitalGainControlMode(int ch);
	Q_INVOKABLE void setRxDigitalGainControlMode(int ch, const QString &val);
	Q_INVOKABLE QString getRxInterfaceGain(int ch);
	Q_INVOKABLE void setRxInterfaceGain(int ch, const QString &val);
	Q_INVOKABLE QString getRxPortEnMode(int ch);
	Q_INVOKABLE void setRxPortEnMode(int ch, const QString &val);
	Q_INVOKABLE QString isRxDynamicAdcSwitchEnabled(int ch);
	Q_INVOKABLE void setRxDynamicAdcSwitchEnabled(int ch, const QString &val);
	Q_INVOKABLE QString getRxBbdcLoopGain(int ch);
	Q_INVOKABLE void setRxBbdcLoopGain(int ch, const QString &val);
	Q_INVOKABLE QString getRxLoFrequency(int ch);
	Q_INVOKABLE void setRxLoFrequency(int ch, const QString &val);
	Q_INVOKABLE QString getRxRssi(int ch);
	Q_INVOKABLE QString getRxSamplingFrequency(int ch);

	// RX Tracking (per-channel, ch=0 or 1)
	Q_INVOKABLE QString isRxQuadratureFicTrackingEnabled(int ch);
	Q_INVOKABLE void setRxQuadratureFicTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxAgcTrackingEnabled(int ch);
	Q_INVOKABLE void setRxAgcTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxBbdcRejectionTrackingEnabled(int ch);
	Q_INVOKABLE void setRxBbdcRejectionTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxQuadraturePolyTrackingEnabled(int ch);
	Q_INVOKABLE void setRxQuadraturePolyTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxHdTrackingEnabled(int ch);
	Q_INVOKABLE void setRxHdTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isRxRssiTrackingEnabled(int ch);
	Q_INVOKABLE void setRxRssiTrackingEnabled(int ch, const QString &val);

	// TX (per-channel, ch=0 or 1)
	Q_INVOKABLE QString getTxAttenuation(int ch);
	Q_INVOKABLE void setTxAttenuation(int ch, const QString &val);
	Q_INVOKABLE QString getTxAttenControlMode(int ch);
	Q_INVOKABLE void setTxAttenControlMode(int ch, const QString &val);
	Q_INVOKABLE QString getTxLoFrequency(int ch);
	Q_INVOKABLE void setTxLoFrequency(int ch, const QString &val);
	Q_INVOKABLE QString getTxNcoFrequency(int ch);
	Q_INVOKABLE void setTxNcoFrequency(int ch, const QString &val);
	Q_INVOKABLE QString getTxRfBandwidth(int ch);
	Q_INVOKABLE QString getTxPortEnMode(int ch);
	Q_INVOKABLE void setTxPortEnMode(int ch, const QString &val);
	Q_INVOKABLE QString getTxEnsmMode(int ch);
	Q_INVOKABLE void setTxEnsmMode(int ch, const QString &val);
	Q_INVOKABLE QString isTxEnabled(int ch);
	Q_INVOKABLE void setTxEnabled(int ch, const QString &val);
	Q_INVOKABLE QString getTxSamplingFrequency(int ch);

	// TX Tracking (per-channel, ch=0 or 1)
	Q_INVOKABLE QString isTxQuadratureTrackingEnabled(int ch);
	Q_INVOKABLE void setTxQuadratureTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isTxCloseLoopGainTrackingEnabled(int ch);
	Q_INVOKABLE void setTxCloseLoopGainTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isTxPaCorrectionTrackingEnabled(int ch);
	Q_INVOKABLE void setTxPaCorrectionTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isTxLoopbackDelayTrackingEnabled(int ch);
	Q_INVOKABLE void setTxLoopbackDelayTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isTxLoLeakageTrackingEnabled(int ch);
	Q_INVOKABLE void setTxLoLeakageTrackingEnabled(int ch, const QString &val);

	// ORX (per-channel, ch=0 or 1)
	Q_INVOKABLE QString getOrxHardwareGain(int ch);
	Q_INVOKABLE void setOrxHardwareGain(int ch, const QString &val);
	Q_INVOKABLE QString isOrxBbdcRejectionEnabled(int ch);
	Q_INVOKABLE void setOrxBbdcRejectionEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isOrxQuadraturePolyTrackingEnabled(int ch);
	Q_INVOKABLE void setOrxQuadraturePolyTrackingEnabled(int ch, const QString &val);
	Q_INVOKABLE QString isOrxEnabled(int ch);
	Q_INVOKABLE void setOrxEnabled(int ch, const QString &val);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString rxKey(int ch, const QString &attr);
	QString txKey(int ch, const QString &attr);
	QString rxLoKey(int ch);
	QString txLoKey(int ch);
	QString orxKey(int ch, const QString &attr);

	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Adrv9002Plugin *m_plugin;
};

} // namespace scopy::adrv9002

#endif // ADRV9002_API_H
