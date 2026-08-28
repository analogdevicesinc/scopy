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

#include <component/device.h>
#include <component/backends/iio/iiotriggerable.h>
#include <component/backends/iio/iiotrigger.h>

#include <qcorotask.h>

using namespace scopy;
using namespace scopy::swiot;

SwiotRuntime::SwiotRuntime(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
{
	m_context = component::Controller::context(m_uri);
}

SwiotRuntime::~SwiotRuntime() { m_context = {}; }

void SwiotRuntime::onIsRuntimeCtxChanged(bool isRuntimeCtx)
{
	if(isRuntimeCtx) {
		writeTriggerDevice();
	}
}

QCoro::Task<void> SwiotRuntime::writeTriggerDevice()
{
	if(!m_context) {
		co_return;
	}
	component::Device *adDev = m_context->findChild<component::Device *>(AD_NAME, Qt::FindDirectChildrenOnly);
	component::Device *trigDev =
		m_context->findChild<component::Device *>(AD_TRIGGER_NAME, Qt::FindDirectChildrenOnly);
	if(!adDev || !trigDev) {
		qDebug(CAT_SWIOT) << "Isn't runtime context";
		co_return;
	}
	auto *triggerable = adDev->findChild<component::iio::IIOTriggerable *>();
	auto *trigger = trigDev->findChild<component::iio::IIOTrigger *>();
	if(!triggerable || !trigger) {
		qDebug(CAT_SWIOT) << "Can't set trigger, missing trigger capability";
		co_return;
	}
	auto r = co_await triggerable->setTriggerAsync(trigger);
	if(r) {
		qDebug(CAT_SWIOT) << "Trigger has been set";
	} else {
		qDebug(CAT_SWIOT) << "Can't set trigger, not in runtime context";
	}
}

void SwiotRuntime::onBackBtnPressed() { Q_EMIT writeModeAttribute("config"); }

#include "moc_swiotruntime.cpp"
