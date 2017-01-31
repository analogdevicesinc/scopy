/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "apiObject.hpp"

#include <QDebug>
#include <QJSEngine>
#include <QMetaProperty>
#include <QSettings>

using namespace adiscope;

ApiObject::ApiObject(enum tool tool) : QObject(nullptr), tool(tool)
{
}

ApiObject::~ApiObject()
{
}

void ApiObject::load()
{
	QSettings settings;

	settings.beginGroup(QString::fromStdString(Filter::tool_name(tool)));

	auto meta = metaObject();
	for (unsigned int i = meta->propertyOffset();
			i < meta->propertyCount(); i++) {
		auto prop = meta->property(i);

		if (prop.isStored() && prop.isWritable()) {
			auto value = settings.value(prop.name());

			qDebug() << "Loading property"
				<< prop.name()
				<< "value" << value;

			if (!value.isNull())
				prop.write(this, value);
		}
	}

	settings.endGroup();
}

void ApiObject::save()
{
	QSettings settings;

	settings.beginGroup(QString::fromStdString(Filter::tool_name(tool)));

	auto meta = metaObject();
	for (unsigned int i = meta->propertyOffset();
			i < meta->propertyCount(); i++) {
		auto prop = meta->property(i);
		auto data = prop.read(this);

		if (prop.isStored() && prop.isReadable() && prop.isWritable()) {

			qDebug() << "Saving property"
				<< prop.name()
				<< "value" << data;

			settings.setValue(prop.name(), data);
		}
	}

	settings.endGroup();
}

void ApiObject::js_register(QJSEngine *engine)
{
	if (engine) {
		auto name = QString::fromStdString(Filter::tool_name(tool));

		engine->globalObject().setProperty(name,
				engine->newQObject(this));
	}
}
