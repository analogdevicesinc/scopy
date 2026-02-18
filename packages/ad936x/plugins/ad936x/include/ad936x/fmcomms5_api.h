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

#ifndef FMCOMMS5_API_H
#define FMCOMMS5_API_H

#include "scopy-ad936x_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad936x {

class Ad936xPlugin;

class SCOPY_AD936X_EXPORT FMCOMMS5_API : public ApiObject
{
	Q_OBJECT
public:
	explicit FMCOMMS5_API(Ad936xPlugin *plugin);
	~FMCOMMS5_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Global device settings (ad9361-phy)
	Q_INVOKABLE QString getEnsmMode();
	Q_INVOKABLE void setEnsmMode(const QString &mode);
	Q_INVOKABLE QString getCalibMode();
	Q_INVOKABLE void setCalibMode(const QString &mode);
	Q_INVOKABLE QString getTrxRateGovernor();
	Q_INVOKABLE void setTrxRateGovernor(const QString &governor);
	Q_INVOKABLE QString getRxPathRates();
	Q_INVOKABLE QString getTxPathRates();
	Q_INVOKABLE QString getXoCorrection();
	Q_INVOKABLE void setXoCorrection(const QString &value);

	// RX chain settings (shared, on ad9361-phy voltage0 input)
	Q_INVOKABLE QString getRxRfBandwidth();
	Q_INVOKABLE void setRxRfBandwidth(const QString &value);
	Q_INVOKABLE QString getRxSamplingFrequency();
	Q_INVOKABLE void setRxSamplingFrequency(const QString &value);
	Q_INVOKABLE QString getRxRfPortSelect();
	Q_INVOKABLE void setRxRfPortSelect(const QString &port);
	Q_INVOKABLE QString isQuadratureTrackingEnabled();
	Q_INVOKABLE void setQuadratureTrackingEnabled(const QString &value);
	Q_INVOKABLE QString isRfDcOffsetTrackingEnabled();
	Q_INVOKABLE void setRfDcOffsetTrackingEnabled(const QString &value);
	Q_INVOKABLE QString isBbDcOffsetTrackingEnabled();
	Q_INVOKABLE void setBbDcOffsetTrackingEnabled(const QString &value);

	// TX chain settings (shared, on ad9361-phy voltage0 output)
	Q_INVOKABLE QString getTxRfBandwidth();
	Q_INVOKABLE void setTxRfBandwidth(const QString &value);
	Q_INVOKABLE QString getTxSamplingFrequency();
	Q_INVOKABLE void setTxSamplingFrequency(const QString &value);
	Q_INVOKABLE QString getTxRfPortSelect();
	Q_INVOKABLE void setTxRfPortSelect(const QString &port);

	// Per-channel RX access (channel 0-1: ad9361-phy, channel 2-3: ad9361-phy-B)
	Q_INVOKABLE QString getRxHardwareGain(int channel);
	Q_INVOKABLE void setRxHardwareGain(int channel, const QString &value);
	Q_INVOKABLE QString getRxGainControlMode(int channel);
	Q_INVOKABLE void setRxGainControlMode(int channel, const QString &mode);
	Q_INVOKABLE QString getRxRssi(int channel);

	// Per-channel TX access (channel 0-1: ad9361-phy, channel 2-3: ad9361-phy-B)
	Q_INVOKABLE QString getTxHardwareGain(int channel);
	Q_INVOKABLE void setTxHardwareGain(int channel, const QString &value);
	Q_INVOKABLE QString getTxRssi(int channel);

	// LO frequencies (device 0: ad9361-phy, device 1: ad9361-phy-B)
	Q_INVOKABLE QString getRxLoFrequency(int device);
	Q_INVOKABLE void setRxLoFrequency(int device, const QString &value);
	Q_INVOKABLE QString getTxLoFrequency(int device);
	Q_INVOKABLE void setTxLoFrequency(int device, const QString &value);

	// Generic widget access
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);
	QString deviceName(int device);
	QString rxChannelKey(int channel, const QString &attr);
	QString txChannelKey(int channel, const QString &attr);

	Ad936xPlugin *m_plugin;
};

} // namespace scopy::ad936x

#endif // FMCOMMS5_API_H
