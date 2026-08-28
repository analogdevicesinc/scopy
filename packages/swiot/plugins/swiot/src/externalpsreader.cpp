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

#include "externalpsreader.h"

#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/attribute.h>
#include <component/attributereader.h>

#include <qcorotask.h>

using namespace scopy::swiot;

ExternalPsReader::ExternalPsReader(QString uri, QString attr, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
	, m_attribute(attr)
	, m_swiot(nullptr)
{
	m_context = component::Controller::context(m_uri);
	m_swiot = m_context ? m_context->findChild<component::Device *>("swiot", Qt::FindDirectChildrenOnly) : nullptr;
	if(!m_swiot) {
		qDebug(CAT_SWIOT) << "Error, no context available for the external ps task.";
	}
}

ExternalPsReader::~ExternalPsReader() { m_context = {}; }

QCoro::Task<void> ExternalPsReader::readPowerSupply()
{
	if(!m_swiot) {
		co_return;
	}
	component::Attribute *attr =
		m_swiot->findChild<component::Attribute *>(m_attribute, Qt::FindDirectChildrenOnly);
	if(!attr || !attr->readCapability()) {
		co_return;
	}
	auto r = co_await attr->readCapability()->readAsync();
	if(!r) {
		qCritical(CAT_SWIOT) << "Error, could not read" << m_attribute << "attribute from swiot device";
		co_return;
	}
	bool ok = false;
	bool extPsuValue = attr->cachedValue().toInt(&ok);
	if(ok) {
		Q_EMIT hasConnectedPowerSupply(extPsuValue);
	}
}

#include "moc_externalpsreader.cpp"
