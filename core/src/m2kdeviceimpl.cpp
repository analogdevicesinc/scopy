/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "m2kdeviceimpl.h"

#include <QLoggingCategory>
#include <QMetaObject>

Q_LOGGING_CATEGORY(CAT_M2K_DEVICEIMPL, "M2kDevice")

using namespace scopy;

M2kDeviceImpl::M2kDeviceImpl(QString param, int maxThreads, QObject *parent)
	: DeviceImpl(param, "iio", parent)
	, m_maxThreads(maxThreads)
{
	acquireGenericContext();
}

void M2kDeviceImpl::acquireGenericContext()
{
	if(m_context) {
		return;
	}
	m_context = component::Controller::connectCtx(m_param, component::BackendKind::Libiiov0, m_maxThreads);
	if(!m_context) {
		qWarning(CAT_M2K_DEVICEIMPL) << "Failed to acquire the generic context for" << m_param;
	}
}

void M2kDeviceImpl::connectDev()
{
	m_context = {};
	m_swapPending = true;

	QMetaObject::invokeMethod(
		this,
		[this]() {
			if(!m_swapPending) {
				return;
			}
			m_swapPending = false;

			m_context =
				component::Controller::connectCtx(m_param, component::BackendKind::M2k, m_maxThreads);
			if(!m_context) {
				qInfo(CAT_M2K_DEVICEIMPL)
					<< "No M2K context for" << m_param << "- falling back to libiio";
				m_context = component::Controller::connectCtx(m_param, component::BackendKind::Libiiov0,
									      m_maxThreads);
			}
			DeviceImpl::connectDev();
		},
		Qt::QueuedConnection);
}

void M2kDeviceImpl::disconnectDev()
{
	m_swapPending = false;
	if(state() != DEV_INIT) {
		DeviceImpl::disconnectDev();
	}
	m_context = {};
}

bool M2kDeviceImpl::verify() { return static_cast<bool>(m_context); }

#include "moc_m2kdeviceimpl.cpp"
