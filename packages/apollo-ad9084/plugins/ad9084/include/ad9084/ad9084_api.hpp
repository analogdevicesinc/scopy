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

#ifndef AD9084_API_HPP
#define AD9084_API_HPP

#include "scopy-ad9084_export.h"
#include <pluginbase/apiobject.h>
#include <QString>

struct iio_device;
class QWidget;

namespace scopy::ad9084 {

class SCOPY_AD9084_EXPORT Ad9084_API : public ApiObject
{
	Q_OBJECT
public:
	explicit Ad9084_API(struct iio_device *dev, QWidget *tool = nullptr, QObject *parent = nullptr);
	~Ad9084_API();

	Q_INVOKABLE void show();
	Q_INVOKABLE void setRxTabEnabled(bool enable);
	Q_INVOKABLE void setTxTabEnabled(bool enable);
	Q_INVOKABLE bool getRxTabEnabled();
	Q_INVOKABLE bool getTxTabEnabled();
	Q_INVOKABLE bool setNco(double freqHz);
	Q_INVOKABLE double getNco();
	Q_INVOKABLE bool setSampleRate(double rateHz);
	Q_INVOKABLE double getSampleRate();
	Q_INVOKABLE bool setEnableChannel(int idx, bool enable);
	Q_INVOKABLE bool getEnableChannel(int idx);
	Q_INVOKABLE bool applyProfile(const QString &path);
	Q_INVOKABLE void refresh();

private:
	struct iio_device *m_dev;
	QWidget *m_tool;
};

} // namespace scopy::ad9084

#endif // AD9084_API_HPP
