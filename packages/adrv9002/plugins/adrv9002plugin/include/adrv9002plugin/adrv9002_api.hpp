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

#ifndef ADRV9002_API_HPP
#define ADRV9002_API_HPP

#include "scopy-adrv9002plugin_export.h"
#include <pluginbase/apiobject.h>
#include <QString>

struct iio_context;
class QWidget;

namespace scopy::adrv9002 {

class SCOPY_ADRV9002PLUGIN_EXPORT Adrv9002_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Adrv9002_API(struct iio_context *ctx, QWidget *tool = nullptr, QObject *parent = nullptr);
	~Adrv9002_API();

	Q_INVOKABLE void show();
	Q_INVOKABLE QString getDriverVersion();
	Q_INVOKABLE double getTemperatureC();
	Q_INVOKABLE bool isInitialCalibrationsSupported();
	Q_INVOKABLE QString getProfileConfig();
	Q_INVOKABLE QString getStreamConfig();
	Q_INVOKABLE bool setRxLo(double freqHz);
	Q_INVOKABLE double getRxLo();
	Q_INVOKABLE bool setTxLo(double freqHz);
	Q_INVOKABLE double getTxLo();
	Q_INVOKABLE bool setRxGain(int chan, double value);
	Q_INVOKABLE double getRxGain(int chan);
	Q_INVOKABLE bool setRxGainMode(int chan, const QString &mode);
	Q_INVOKABLE QString getRxGainMode(int chan);
	Q_INVOKABLE bool setRxPowerdown(int chan, bool powerdown);
	Q_INVOKABLE bool getRxPowerdown(int chan);
	Q_INVOKABLE bool setTxAtten(int chan, double value);
	Q_INVOKABLE double getTxAtten(int chan);
	Q_INVOKABLE bool setTxAttenMode(int chan, const QString &mode);
	Q_INVOKABLE QString getTxAttenMode(int chan);
	Q_INVOKABLE bool setTxPowerdown(int chan, bool powerdown);
	Q_INVOKABLE bool getTxPowerdown(int chan);
	Q_INVOKABLE bool setRxEnsmMode(int chan, const QString &mode);
	Q_INVOKABLE QString getRxEnsmMode(int chan);
	Q_INVOKABLE bool setTxEnsmMode(int chan, const QString &mode);
	Q_INVOKABLE QString getTxEnsmMode(int chan);
	Q_INVOKABLE double getRxDecimatedPower(int chan);
	Q_INVOKABLE double getRxBandwidth(int chan);
	Q_INVOKABLE double getTxBandwidth(int chan);
	Q_INVOKABLE double getRxSampleRate(int chan);
	Q_INVOKABLE double getTxSampleRate(int chan);
	Q_INVOKABLE bool setSampleRate(double rateHz);
	Q_INVOKABLE double getSampleRate();
	Q_INVOKABLE bool setRfBandwidth(double bwHz);
	Q_INVOKABLE double getRfBandwidth();
	Q_INVOKABLE bool setOrxGain(int chan, double value);
	Q_INVOKABLE double getOrxGain(int chan);
	Q_INVOKABLE bool setOrxPowerdown(int chan, bool powerdown);
	Q_INVOKABLE bool getOrxPowerdown(int chan);
	Q_INVOKABLE bool setBbdcRejection(int chan, bool enable);
	Q_INVOKABLE bool getBbdcRejection(int chan);
	Q_INVOKABLE bool applyProfile(const QString &path);
	Q_INVOKABLE void refresh();

private:
	struct iio_context *m_ctx;
	QWidget *m_tool;
};

} // namespace scopy::adrv9002

#endif // ADRV9002_API_HPP
