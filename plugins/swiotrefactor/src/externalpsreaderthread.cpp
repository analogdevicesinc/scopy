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

#include "swiot_logging_categories.h"
#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <utility>

using namespace scopy::swiotrefactor;

ExternalPsReaderThread::ExternalPsReaderThread(QString uri, QString attr, QObject *parent)
	: QThread(parent)
	, m_uri(uri)
	, m_attribute(attr)
{
	m_conn = ConnectionProvider::open(m_uri);
	if(!m_conn) {
		qDebug(CAT_SWIOT) << "Error, no context available for the external ps task.";
	}
	connect(m_conn, &Connection::aboutToBeDestroyed, this, [=, this]() { m_conn = nullptr; });
}

ExternalPsReaderThread::~ExternalPsReaderThread()
{
	if(m_conn) {
		m_conn = nullptr;
		ConnectionProvider::close(m_uri);
	}
}

void ExternalPsReaderThread::run()
{
	if(!m_conn) {
		return;
	}
	if(isInterruptionRequested()) {
		return;
	}

	iio_device *swiotDevice = iio_context_find_device(m_conn->context(), "swiot");
	if(swiotDevice) {
		IioDeviceAttributeRead *iioAttrRead =
			new IioDeviceAttributeRead(swiotDevice, m_attribute.toStdString().c_str(), nullptr, true);

		connect(
			iioAttrRead, &scopy::Command::finished, this,
			[=, this](scopy::Command *cmd) {
				IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
				if(!tcmd) {
					return;
				}
				if(tcmd->getReturnCode() >= 0) {
					char *extPsu = tcmd->getResult();
					bool ok = false;
					bool extPsuValue = QString(extPsu).toInt(&ok);
					if(ok) {
						Q_EMIT hasConnectedPowerSupply(extPsuValue);
					}
				} else {
					qCritical(CAT_SWIOT) << "Error, could not read ext_psu attribute from swiot "
								"device, error code"
							     << tcmd->getReturnCode();
				}
			},
			Qt::QueuedConnection);

		m_conn->commandQueue()->enqueue(iioAttrRead);
	}
}

#include "moc_externalpsreaderthread.cpp"
