/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef DMM_HPP
#define DMM_HPP

#include <QList>
#include <QMap>
#include <QObject>
#include "datamonitor/dmmdatamonitormodel.hpp"
#include "iio.h"
#include "scopy-datalogger_export.h"
#include "iioutil/iiounits.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DMM : public QObject
{
	Q_OBJECT
public:
	explicit DMM(QObject *parent = nullptr);

	QList<DmmDataMonitorModel *> getDmmMonitors(iio_context *ctx);
	bool isDMMCompatible(iio_channel *chn);
	bool isHwmon(iio_device *dev, iio_channel *chn);
	bool iioChannelHasAttribute(iio_channel *chn, std::string const &attr);
	void generateDictionaries();

	QMap<iio_chan_type, IIOUnit> iioDevices() const;
	QMap<hwmon_chan_type, IIOUnit> hwmonDevices() const;

private:
	QMap<iio_chan_type, IIOUnit> m_iioDevices;
	QMap<hwmon_chan_type, IIOUnit> m_hwmonDevices;
};
} // namespace datamonitor
} // namespace scopy
#endif // DMM_HPP
