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

#include "swiotidentifytask.h"

#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/attribute.h>
#include <component/attributewriter.h>

#include <qcorotask.h>

using namespace scopy::swiot;

SwiotIdentifyTask::SwiotIdentifyTask(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
	, m_swiot(nullptr)
{
	m_context = component::Controller::context(m_uri);
	m_swiot = m_context ? m_context->findChild<component::Device *>("swiot", Qt::FindDirectChildrenOnly) : nullptr;
	if(!m_swiot) {
		qDebug(CAT_SWIOT) << "Error, no context available for the identify task.";
	}
}

SwiotIdentifyTask::~SwiotIdentifyTask() { m_context = {}; }

QCoro::Task<void> SwiotIdentifyTask::identify()
{
	if(!m_swiot) {
		co_return;
	}
	component::Attribute *attr = m_swiot->findChild<component::Attribute *>("identify", Qt::FindDirectChildrenOnly);
	if(!attr || !attr->writeCapability()) {
		qCritical(CAT_SWIOT) << "Error, could not identify swiot.";
		co_return;
	}
	auto r = co_await attr->writeCapability()->writeAsync("1");
	if(!r) {
		qCritical(CAT_SWIOT) << "Error, could not identify swiot.";
	}
}

#include "moc_swiotidentifytask.cpp"
