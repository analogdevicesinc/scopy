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

#include "swiotruntime.h"

#include "swiot_logging_categories.h"
#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiodevicesettrigger.h>
#include <string>

using namespace scopy::swiotrefactor;

SwiotRuntime::SwiotRuntime(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
{
	Connection *conn = ConnectionProvider::open(m_uri);
	m_iioCtx = conn->context();
	m_cmdQueue = conn->commandQueue();
	createDevicesMap();
}

SwiotRuntime::~SwiotRuntime() { ConnectionProvider::close(m_uri); }

void SwiotRuntime::onIsRuntimeCtxChanged(bool isRuntimeCtx)
{
	if(isRuntimeCtx) {
		writeTriggerDevice();
	}
}

void SwiotRuntime::writeTriggerDevice()
{
	if(m_iioDevices.contains(AD_TRIGGER_NAME)) {
		Command *setTriggerCommand =
			new IioDeviceSetTrigger(m_iioDevices[AD_NAME], m_iioDevices[AD_TRIGGER_NAME], nullptr);
		connect(setTriggerCommand, &scopy::Command::finished, this, &SwiotRuntime::setTriggerCommandFinished,
			Qt::QueuedConnection);
		m_cmdQueue->enqueue(setTriggerCommand);
	} else {
		qDebug(CAT_SWIOT) << "Isn't runtime context";
	}
}

void SwiotRuntime::setTriggerCommandFinished(scopy::Command *cmd)
{
	IioDeviceSetTrigger *tcmd = dynamic_cast<IioDeviceSetTrigger *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		qDebug(CAT_SWIOT) << "Trigger has been set: " + QString::number(tcmd->getReturnCode());
	} else {
		qDebug(CAT_SWIOT) << "Can't set trigger, not in runtime context";
	}
}

void SwiotRuntime::createDevicesMap()
{
	if(m_iioCtx) {
		int devicesNumber = iio_context_get_devices_count(m_iioCtx);
		m_iioDevices.clear();
		for(int i = 0; i < devicesNumber; i++) {
			struct iio_device *iioDev = iio_context_get_device(m_iioCtx, i);
			if(iioDev) {
				QString deviceName = QString(iio_device_get_name(iioDev));
				m_iioDevices[deviceName] = iioDev;
			}
		}
	}
}

void SwiotRuntime::onBackBtnPressed() { Q_EMIT writeModeAttribute("config"); }
