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

#include "m2kreadtemperaturetask.h"

#include "m2kcommon.h"

#include <QDebug>

#include <iioutil/connectionprovider.h>

using namespace scopy::m2k;

M2kReadTemperatureTask::M2kReadTemperatureTask(QString uri)
	: QThread()
{
	m_uri = uri;
}

void M2kReadTemperatureTask::run()
{
	double val, scale, offset, temperature;
	iio_device *dev;
	iio_channel *ch;
	int ret;

	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	if(!conn)
		goto finish;
	dev = iio_context_find_device(conn->context(), "ad9963");
	if(!dev)
		goto finish;
	ch = iio_device_find_channel(dev, "temp0", false);
	if(!ch)
		goto finish;

	ret = iio_channel_attr_read_double(ch, "raw", &val);
	if(ret)
		goto finish;
	iio_channel_attr_read_double(ch, "scale", &scale);
	if(ret)
		goto finish;
	iio_channel_attr_read_double(ch, "offset", &offset);
	if(ret)
		goto finish;

	temperature = (val + offset) * scale / 1000;
	qDebug(CAT_M2KPLUGIN) << "Read temperature" << temperature;

	Q_EMIT newTemperature(temperature);

finish:
	if(conn)
		ConnectionProvider::GetInstance()->close(m_uri);
	return;
}
