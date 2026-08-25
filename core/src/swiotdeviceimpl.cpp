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

#include "swiotdeviceimpl.h"

using namespace scopy;

void SWIOTDeviceImpl::connectDev()
{
	if(!m_context) {
		m_context = component::Controller::connectCtx(m_param, component::BackendKind::Libiiov0);
	}
	IIODeviceImpl::connectDev();
}

void SWIOTDeviceImpl::disconnectDev()
{
	IIODeviceImpl::disconnectDev();
	m_context = component::ContextHandle{};
}

#include "moc_swiotdeviceimpl.cpp"
