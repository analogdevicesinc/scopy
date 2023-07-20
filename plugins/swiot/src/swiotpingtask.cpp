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


#include "swiotpingtask.h"
#include <QDebug>
#include "src/swiot_logging_categories.h"
#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodevicegettrigger.h>

using namespace scopy;
using namespace scopy::swiot;
SwiotPingTask::SwiotPingTask(iio_context *c ,QObject *parent) : QThread(parent), c(c) {}

SwiotPingTask::~SwiotPingTask() {}

void SwiotPingTask::run() {

	enabled = true;
	CommandQueue *commandQueue = CommandQueueProvider::GetInstance()->open(c);
	if (!commandQueue || !c) {
		Q_EMIT pingFailed();
		return;
	}

	auto dev = iio_context_find_device(c, "sw_trig");

	if (dev) {
		Command *getTriggerCommand = new IioDeviceGetTrigger(dev, nullptr);
		connect(getTriggerCommand, &scopy::Command::finished, this, &SwiotPingTask::getTriggerCommandFinished, Qt::QueuedConnection);
		commandQueue->enqueue(getTriggerCommand);
	} else {
		CommandQueueProvider::GetInstance()->close(c);
	}
}

void SwiotPingTask::getTriggerCommandFinished(scopy::Command *cmd)
{
	IioDeviceGetTrigger *tcmd = dynamic_cast<IioDeviceGetTrigger*>(cmd);
	if (!tcmd) {
		CommandQueueProvider::GetInstance()->close(c);
		return;
	}
	int ret = tcmd->getReturnCode();
	if (ret>= 0 || ret == -ENOENT) {
		Q_EMIT pingSuccess();
	} else {
		Q_EMIT pingFailed();
	}
	CommandQueueProvider::GetInstance()->close(c);
}

#include "moc_swiotpingtask.cpp"
