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

#include "contextattrdatastrategy.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CONTEXT_ATTR_DATA_STRATEGY, "ContextAttrDataStrategy")
using namespace scopy;

ContextAttrDataStrategy::ContextAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent)
	: QObject(parent)
{
	m_recipe = recipe;
	m_optionalData = "";
}

ContextAttrDataStrategy::ContextAttrDataStrategy() {}

QString ContextAttrDataStrategy::data() { return m_data; }

QString ContextAttrDataStrategy::optionalData() { return m_optionalData; }

int ContextAttrDataStrategy::write(QString data)
{
	qInfo(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "ContextAttrDataStrategy::save called, but the attr is read only";
	return 0;
}

QPair<QString, QString> ContextAttrDataStrategy::read()
{
	if(m_recipe.context == nullptr || m_recipe.data == "") {
		qWarning(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "Invalid arguments, cannot read any data";
		return {};
	}

	const char *value = iio_context_get_attr_value(m_recipe.context, m_recipe.data.toStdString().c_str());

	if(value == nullptr) {
		qWarning(CAT_CONTEXT_ATTR_DATA_STRATEGY) << "Could not read value from context attr" << m_recipe.data;
		return {};
	}

	// There is no return code for the context attr read op, just return 0
	m_previousData = m_data;
	m_data = value;

	return {m_data, m_optionalData};
}

void ContextAttrDataStrategy::writeAsync(QString data)
{
	int res = write(data);
	// The status needs to be emitted in order to keep the iio-widget from getting stuck in
	// the "Busy" state
	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, res, false);
	readAsync();
}

void ContextAttrDataStrategy::readAsync()
{
	read();

	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_previousData, m_data, 0, true);
	Q_EMIT sendData(m_data, "");
}

#include "moc_contextattrdatastrategy.cpp"
