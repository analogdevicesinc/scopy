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


#include "externalpsreaderthread.h"
#include <iioutil/contextprovider.h>
#include "src/swiot_logging_categories.h"

#include <utility>

using namespace scopy::swiot;

ExternalPsReaderThread::ExternalPsReaderThread(QString uri, QString attr)
	: QThread()
	, m_uri(uri)
	, m_attribute(attr)
{}

void ExternalPsReaderThread::run() {
	bool value = false;
	iio_context *ctx = ContextProvider::GetInstance()->open(m_uri);
	if (!ctx) {
		return;
	}
	if (isInterruptionRequested()) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	iio_device *swiotDevice = iio_context_find_device(ctx, "swiot");
	if (swiotDevice) {
		ssize_t res = iio_device_attr_read_bool(swiotDevice, m_attribute.toStdString().c_str(), &value);
		if (res < 0) {
			qCritical(CAT_SWIOT) << "Error, could not read ext_psu attribute from swiot device, error code" << res;
		} else {
			qCritical(CAT_SWIOT) << "testing ps" << res;
			Q_EMIT hasConnectedPowerSupply(value);
		}
	}
	ContextProvider::GetInstance()->close(m_uri);
}

#include "moc_externalpsreaderthread.cpp"
