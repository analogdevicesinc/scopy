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

#include "src/swiot_logging_categories.h"

#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>

using namespace scopy::swiot;

SwiotSwitchCtxTask::SwiotSwitchCtxTask(QString uri, bool wasRuntime)
	: QThread()
	, m_uri(uri)
	, m_wasRuntime(wasRuntime)
	, m_conn(nullptr)
{
	connect(this, &SwiotSwitchCtxTask::contextSwitchFailed, this, [=, this]() { start(); });
}

SwiotSwitchCtxTask::~SwiotSwitchCtxTask() {}

void SwiotSwitchCtxTask::run()
{
	m_conn = ConnectionProvider::open(m_uri);
	if(!m_conn) {
		return;
	}
	connect(m_conn, &Connection::aboutToBeDestroyed, this, [=, this]() {
		if(m_conn) {
			ConnectionProvider::close(m_uri);
			m_conn = nullptr;
		}
	});

	if(isInterruptionRequested()) {
		ConnectionProvider::close(m_uri);
		m_conn = nullptr;
		return;
	}

	iio_device *swiotDevice = iio_context_find_device(m_conn->context(), "swiot");
	if(swiotDevice) {
		IioDeviceAttributeRead *iioAttrRead = new IioDeviceAttributeRead(swiotDevice, "mode", nullptr, true);

		connect(iioAttrRead, &scopy::Command::finished, this, &SwiotSwitchCtxTask::onReadModeFinished,
			Qt::QueuedConnection);
		connect(m_conn, &scopy::Connection::aboutToBeDestroyed, this, [=, this]() {
			disconnect(iioAttrRead, &scopy::Command::finished, this,
				   &SwiotSwitchCtxTask::onReadModeFinished);
		});

		m_conn->commandQueue()->enqueue(iioAttrRead);
	} else {
		ConnectionProvider::close(m_uri);
	}
}

void SwiotSwitchCtxTask::onReadModeFinished(scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		Q_EMIT contextSwitchFailed();
		ConnectionProvider::close(m_uri);
		return;
	}
	char *readMode = tcmd->getResult();
	if(tcmd->getReturnCode() >= 0) {
		if((m_wasRuntime && (strcmp(readMode, "config") == 0)) ||
		   (!m_wasRuntime && (strcmp(readMode, "runtime") == 0))) {
			qDebug(CAT_SWIOT) << "Context has been changed";
			if(m_conn) {
				m_conn = nullptr;
				ConnectionProvider::close(m_uri);
			}
			Q_EMIT contextSwitched();
		}
	} else if(m_conn) {
		Q_EMIT contextSwitchFailed();
		m_conn = nullptr;
		ConnectionProvider::close(m_uri);
	}
}
