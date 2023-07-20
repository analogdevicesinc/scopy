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


#include "swiotidentifytask.h"
#include <iio.h>
#include <iioutil/contextprovider.h>
#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;
SwiotIdentifyTask::SwiotIdentifyTask(QString uri, QObject *parent) : QThread(parent), m_uri(uri) {}

SwiotIdentifyTask::~SwiotIdentifyTask() {}

void SwiotIdentifyTask::run() {
	iio_context *ctx = ContextProvider::GetInstance()->open(m_uri);

	if (!ctx) {
		return;
	}
	if (isInterruptionRequested()) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	CommandQueue *commandQueue = CommandQueueProvider::GetInstance()->open(ctx);
	if (!commandQueue) {
		ContextProvider::GetInstance()->close(m_uri);
		return;
	}
	iio_device *swiotDevice = iio_context_find_device(ctx, "swiot");

	if (swiotDevice) {
		IioDeviceAttributeWrite *iioAttrWrite = new IioDeviceAttributeWrite(swiotDevice, "identify", "1", nullptr, true);

		connect(iioAttrWrite, &scopy::Command::finished, this, [=, this](scopy::Command *cmd) {
			IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite*>(cmd);
			if (!tcmd) {
				CommandQueueProvider::GetInstance()->close(ctx);
				ContextProvider::GetInstance()->close(m_uri);
				return;
			}
			if (tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT) << "Error, could not identify swiot, error code" << tcmd->getReturnCode();
			}
			CommandQueueProvider::GetInstance()->close(ctx);
			ContextProvider::GetInstance()->close(m_uri);
		}, Qt::QueuedConnection);

		commandQueue->enqueue(iioAttrWrite);
	} else {
		CommandQueueProvider::GetInstance()->close(ctx);
		ContextProvider::GetInstance()->close(m_uri);
	}
}
