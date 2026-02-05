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

#ifndef AD936X_API_HPP
#define AD936X_API_HPP

#include "scopy-ad936x_export.h"
#include <pluginbase/apiobject.h>
#include <QString>

struct iio_context;
class QWidget;

namespace scopy::ad936x {
class Ad936xPlugin;

class SCOPY_AD936X_EXPORT Ad936x_API : public ApiObject
{
	Q_OBJECT
public:
	enum class ToolRole
	{
		Main,
		Advanced
	};

	explicit Ad936x_API(Ad936xPlugin *plugin);
	~Ad936x_API();

	void setToolRole(ToolRole role);

	Q_INVOKABLE bool setRxLo(double freqHz);
	Q_INVOKABLE double getRxLo();
	Q_INVOKABLE bool setTxLo(double freqHz);
	Q_INVOKABLE double getTxLo();
	Q_INVOKABLE bool setSampleRate(double rateHz);
	Q_INVOKABLE double getSampleRate();
	Q_INVOKABLE bool setRfBandwidth(double bwHz);
	Q_INVOKABLE double getRfBandwidth();
	Q_INVOKABLE bool setRxBandwidth(double bwHz);
	Q_INVOKABLE double getRxBandwidth();
	Q_INVOKABLE bool setTxBandwidth(double bwHz);
	Q_INVOKABLE double getTxBandwidth();
	Q_INVOKABLE bool setGain(int chan, double value);
	Q_INVOKABLE double getGain(int chan);
	Q_INVOKABLE bool setEnsmMode(const QString &mode);
	Q_INVOKABLE QString getEnsmMode();
	Q_INVOKABLE bool setCalibrationMode(const QString &mode);
	Q_INVOKABLE QString getCalibrationMode();
	Q_INVOKABLE bool setRateGovernor(const QString &mode);
	Q_INVOKABLE QString getRateGovernor();
	Q_INVOKABLE void refresh();

private:
	struct iio_context *context() const;
	QWidget *toolWidget() const;

	Ad936xPlugin *m_plugin;
	ToolRole m_role = ToolRole::Main;
};

} // namespace scopy::ad936x

#endif // AD936X_API_HPP
