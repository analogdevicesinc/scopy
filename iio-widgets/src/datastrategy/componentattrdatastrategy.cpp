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

#include "datastrategy/componentattrdatastrategy.h"

#include <QLoggingCategory>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>
#include <qcorotask.h>

Q_LOGGING_CATEGORY(CAT_COMPONENT_DATA_STRATEGY, "ComponentAttrDataStrategy")
using namespace scopy;

ComponentAttrDataStrategy::ComponentAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: QObject(parent)
	, m_attr(recipe.attribute)
	, m_optionalData(recipe.constDataOptions)
{
	m_recipe = recipe;
	if(!m_attr) {
		qWarning(CAT_COMPONENT_DATA_STRATEGY) << "Null attribute, this strategy will do nothing";
		return;
	}

	m_data = m_attr->cachedValue();

	// Sole sendData path: fires for our own value-changing reads AND for writes from
	// any other widget bound to the same Attribute (automatic cross-widget sync).
	connect(
		m_attr, &component::Attribute::valueChanged, this,
		[this](const QString &value) {
			m_data = value;
			Q_EMIT sendData(m_data, m_optionalData);
		},
		Qt::DirectConnection);
}

ComponentAttrDataStrategy::~ComponentAttrDataStrategy() {}

QString ComponentAttrDataStrategy::data() { return m_data; }

QString ComponentAttrDataStrategy::optionalData() { return m_optionalData; }

void ComponentAttrDataStrategy::readAsync()
{
	if(!m_attr) {
		return;
	}
	component::AttributeReader *reader = m_attr->readCapability();
	if(!reader) {
		qWarning(CAT_COMPONENT_DATA_STRATEGY) << "Attribute has no read capability";
		return;
	}

	// Push the settled value to the GUI unconditionally. valueChanged only fires when
	// the value actually differs, so a read returning the already-cached value (any
	// widget binding to an attribute another widget already read - the tree is shared
	// and Controller-owned) would otherwise leave this widget blank. Mirrors
	// ChannelAttrDataStrategy::readAsync()'s always-sendData contract.
	connect(
		reader, &component::AttributeReader::readSucceeded, this,
		[this](scopy::Result<QByteArray> &) {
			m_data = m_attr->cachedValue();
			Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, m_data, 0, true);
			Q_EMIT sendData(m_data, m_optionalData);
		},
		Qt::SingleShotConnection);
	connect(
		reader, &component::AttributeReader::readFailed, this,
		[this](const scopy::Error &error) {
			Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, m_data, error.errorCode(), true);
		},
		Qt::SingleShotConnection);

	reader->readAsync();
}

void ComponentAttrDataStrategy::writeAsync(QString data)
{
	if(!m_attr) {
		return;
	}
	component::AttributeWriter *writer = m_attr->writeCapability();
	if(!writer) {
		qWarning(CAT_COMPONENT_DATA_STRATEGY) << "Attribute" << m_recipe.data << "is read-only";
		Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, -EINVAL, false);
		return;
	}

	Q_EMIT aboutToWrite(m_data, data);
	connect(
		writer, &component::AttributeWriter::writeSucceeded, this,
		[this, data]() {
			// On success the Attribute triggers a read-back -> valueChanged -> sendData.
			Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, 0, false);
		},
		Qt::SingleShotConnection);
	connect(
		writer, &component::AttributeWriter::writeFailed, this,
		[this, data](const scopy::Error &error) {
			Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, error.errorCode(), false);
		},
		Qt::SingleShotConnection);

	writer->writeAsync(data); // eager QCoro task, fire-and-forget
}

QPair<QString, QString> ComponentAttrDataStrategy::read()
{
	if(!m_attr) {
		return {};
	}
	if(component::AttributeReader *reader = m_attr->readCapability()) {
		// Synchronous contract: block until the value settles (valueChanged fires
		// during the wait if the value changed, refreshing the UI via sendData).
		QCoro::waitFor(reader->readAsync());
	}
	m_data = m_attr->cachedValue();
	return {m_data, m_optionalData};
}

int ComponentAttrDataStrategy::write(QString data)
{
	if(!m_attr) {
		return -EINVAL;
	}
	component::AttributeWriter *writer = m_attr->writeCapability();
	if(!writer) {
		qWarning(CAT_COMPONENT_DATA_STRATEGY) << "Attribute" << m_recipe.data << "is read-only";
		return -EINVAL;
	}

	Q_EMIT aboutToWrite(m_data, data);
	Result<void> result = QCoro::waitFor(writer->writeAsync(data));
	if(!result) {
		qWarning(CAT_COMPONENT_DATA_STRATEGY) << "Cannot write" << data << "to" << m_recipe.data;
		return -result.error().errorCode();
	}
	return 0;
}

#include "moc_componentattrdatastrategy.cpp"
