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

#include "cmdqpingtask.h"

#include <QLoggingCategory>
#include "iiocommand/iiodevicegettrigger.h"

Q_LOGGING_CATEGORY(CAT_CMDQPING, "CmdQPingTask")

using namespace scopy;
CmdQPingTask::CmdQPingTask(Connection *conn, QObject *parent)
	: PingTask(parent)
{
	c = conn;
}

CmdQPingTask::CmdQPingTask(Connection *conn, QString pingDevice, QObject *parent)
	: PingTask(parent)
{
	c = conn;
	m_pingDevice = pingDevice;
}

CmdQPingTask::~CmdQPingTask() {}

void CmdQPingTask::run()
{
	if(!c) {
		Q_EMIT pingFailed();
		return;
	}
	int lastCmdInterval = c->commandQueue()->lastCmdTime().msecsTo(QTime::currentTime());
	if(lastCmdInterval < MS_TO_WAIT) {
		return;
	}
	bool pingStatus = ping();
	if(!pingStatus) {
		Q_EMIT pingFailed();
		return;
	}
}

bool CmdQPingTask::ping()
{
	auto dev = (!m_pingDevice.isEmpty()) ? iio_context_find_device(c->context(), m_pingDevice.toStdString().c_str())
					     : iio_context_find_device(c->context(), 0);

	if(!dev) {
		qWarning(CAT_CMDQPING) << "Device not found!";
		return false;
	}
	Command *getTriggerCommand = new IioDeviceGetTrigger(dev, nullptr);
	connect(getTriggerCommand, &scopy::Command::finished, this, &CmdQPingTask::getTriggerCommandFinished,
		Qt::QueuedConnection);
	c->commandQueue()->enqueue(getTriggerCommand);
	return true;
}

void CmdQPingTask::getTriggerCommandFinished(scopy::Command *cmd)
{
	IioDeviceGetTrigger *tcmd = dynamic_cast<IioDeviceGetTrigger *>(cmd);
	if(!tcmd) {
		return;
	}
	int ret = tcmd->getReturnCode();
	if(ret >= 0 || ret == -ENOENT) {
		Q_EMIT pingSuccess();
	} else {
		Q_EMIT pingFailed();
	}
}

#include "moc_cmdqpingtask.cpp"
