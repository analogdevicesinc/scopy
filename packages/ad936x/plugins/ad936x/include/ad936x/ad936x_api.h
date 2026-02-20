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

#ifndef AD936X_API_H
#define AD936X_API_H

#include "scopy-ad936x_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad936x {

class Ad936xPlugin;

class SCOPY_AD936X_EXPORT AD936X_API : public ApiObject
{
	Q_OBJECT
public:
	explicit AD936X_API(Ad936xPlugin *plugin);
	~AD936X_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// Global device settings
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

	// RX chain (voltage0 input)
	Q_INVOKABLE QString getRxRfBandwidth();
	Q_INVOKABLE void setRxRfBandwidth(const QString &value);
	Q_INVOKABLE QString getRxSamplingFrequency();
	Q_INVOKABLE void setRxSamplingFrequency(const QString &value);
	Q_INVOKABLE QString getRxRfPortSelect();
	Q_INVOKABLE void setRxRfPortSelect(const QString &port);
	Q_INVOKABLE QString getRxLoFrequency();
	Q_INVOKABLE void setRxLoFrequency(const QString &value);
	Q_INVOKABLE QString getRxHardwareGain(int channel);
	Q_INVOKABLE void setRxHardwareGain(int channel, const QString &value);
	Q_INVOKABLE QString getRxGainControlMode(int channel);
	Q_INVOKABLE void setRxGainControlMode(int channel, const QString &mode);
	Q_INVOKABLE QString getRxRssi(int channel);
	Q_INVOKABLE QString isQuadratureTrackingEnabled();
	Q_INVOKABLE void setQuadratureTrackingEnabled(const QString &value);
	Q_INVOKABLE QString isRfDcOffsetTrackingEnabled();
	Q_INVOKABLE void setRfDcOffsetTrackingEnabled(const QString &value);
	Q_INVOKABLE QString isBbDcOffsetTrackingEnabled();
	Q_INVOKABLE void setBbDcOffsetTrackingEnabled(const QString &value);

	// TX chain (voltage0 output)
	Q_INVOKABLE QString getTxRfBandwidth();
	Q_INVOKABLE void setTxRfBandwidth(const QString &value);
	Q_INVOKABLE QString getTxSamplingFrequency();
	Q_INVOKABLE void setTxSamplingFrequency(const QString &value);
	Q_INVOKABLE QString getTxRfPortSelect();
	Q_INVOKABLE void setTxRfPortSelect(const QString &port);
	Q_INVOKABLE QString getTxLoFrequency();
	Q_INVOKABLE void setTxLoFrequency(const QString &value);
	Q_INVOKABLE QString getTxHardwareGain(int channel);
	Q_INVOKABLE void setTxHardwareGain(int channel, const QString &value);
	Q_INVOKABLE QString getTxRssi(int channel);

	// Generic widget access (covers all registered widgets including advanced)
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Utility
	Q_INVOKABLE void refresh();
	Q_INVOKABLE bool isFmcomms5();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Ad936xPlugin *m_plugin;
};

} // namespace scopy::ad936x

#endif // AD936X_API_H
