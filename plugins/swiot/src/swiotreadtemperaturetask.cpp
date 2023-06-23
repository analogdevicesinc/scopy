/*
 * Copyright (c) 2023 Analog Devices Inc.
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


#include "swiotreadtemperaturetask.h"
#include "src/swiot_logging_categories.h"

#include <utility>
#include <iioutil/contextprovider.h>

#define DEVICE_NAME "adt75"
#define CHANNEL_NAME "temp"

using namespace scopy::swiot;


SwiotReadTemperatureTask::SwiotReadTemperatureTask(QString  uri, QObject* parent) :
	QThread(parent),
	m_uri(std::move(uri))
{}

void SwiotReadTemperatureTask::run() {
	double raw, scale, offset, temperature;
	int result;

	iio_context *context = ContextProvider::GetInstance()->open(m_uri);

	if (!context) {
		qCritical(CAT_SWIOT) << "Error, empty context received by temperature task.";
		return;
	}

	struct iio_device* device = iio_context_find_device(context, DEVICE_NAME);
	if (!device) {
		qCritical(CAT_SWIOT) << "Error, could not find" << DEVICE_NAME << "from the given context. Temperature not available.";
		return;
	}

	struct iio_channel* channel = iio_device_find_channel(device, CHANNEL_NAME, false);
	if (!channel) {
		qCritical(CAT_SWIOT) << "Error, could not find channel " << CHANNEL_NAME << "from device" << DEVICE_NAME << ". Temperature not available.";
		return;
	}

	result = iio_channel_attr_read_double(channel, "raw", &raw);
	if (result < 0) {
		qCritical(CAT_SWIOT) << "Error, could not read \"raw\" attribute from " << DEVICE_NAME << ". Temperature not available.";
		return;
	}

	result = iio_channel_attr_read_double(channel, "scale", &scale);
	if (result < 0) {
		qCritical(CAT_SWIOT) << "Error, could not read \"scale\" attribute from " << DEVICE_NAME << ". Temperature not available.";
		return;
	}

	result = iio_channel_attr_read_double(channel, "offset", &offset);
	if (result < 0) {
		qCritical(CAT_SWIOT) << "Error, could not read \"offset\" attribute from " << DEVICE_NAME << ". Temperature not available.";
		return;
	}

	temperature = (raw + offset) * scale / 1000;
	qDebug(CAT_SWIOT) << "Read temperature value of" << temperature;

	Q_EMIT newTemperature(temperature);
}

#include "moc_swiotreadtemperaturetask.cpp"
