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


#include "swiotswitchctxtask.h"
#include <iioutil/contextprovider.h>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

SwiotSwitchCtxTask::SwiotSwitchCtxTask(QString uri, bool wasRuntime)
	: QThread()
	, m_uri(uri)
	, m_wasRuntime(wasRuntime)
{}

void SwiotSwitchCtxTask::run()
{
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
		char mode[64];
		ssize_t result = iio_device_attr_read(swiotDevice, "mode", mode, 64);
		if (result >= 0) {
			if (m_wasRuntime && (strcmp(mode,"runtime") == 0)) {
				return;
			}

			if (!m_wasRuntime && (strcmp(mode,"config") == 0)) {
				return;
			}
			//need to be changed to CAT_SWIOTPLUGIN
			qDebug(CAT_SWIOT)<<"Context has been changed";
			Q_EMIT contextSwitched();
		}
	}
	ContextProvider::GetInstance()->close(m_uri);
}

