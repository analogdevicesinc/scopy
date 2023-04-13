/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "apiobject.h"

#include <QDebug>
#include <QList>
#include <QMetaProperty>
#include <QSettings>
#include <QLoggingCategory>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(CAT_APIOBJECT,"ApiObject");
using namespace scopy;

ApiObject::ApiObject(QObject *parent) : QObject(parent)
{
}

ApiObject::~ApiObject()
{
}

template <typename T> void ApiObject::save(QSettings& settings,
		const QString& prop, const QList<T>& list)
{
	settings.beginWriteArray(prop, list.size());

	for (int i = 0; i < list.size(); i++) {
		settings.setArrayIndex(i);
		settings.setValue("idx", QVariant(list.at(i)));
	}

	settings.endArray();
}

void ApiObject::save(QSettings& settings, const QString& prop,
		const QVariantList& list)
{
	settings.beginWriteArray(prop, list.size());

	for (int i = 0; i < list.size(); i++) {
		settings.setArrayIndex(i);
		save_nogroup(list.at(i).value<ApiObject *>(), settings);
	}

	settings.endArray();
}

template <typename T> QList<T> ApiObject::load(
		QSettings& settings, const QString& prop)
{
	int nb = settings.beginReadArray(prop);
	QList<T> list;

	for (int i = 0; i < nb; i++) {
		settings.setArrayIndex(i);
		list.append(settings.value("idx").value<T>());
	}

	settings.endArray();
	return list;
}

void ApiObject::load(QSettings& settings, const QString& prop,
		const QVariantList& list)
{
	int nb = settings.beginReadArray(prop);

	for (int i = 0; i < nb; i++) {
		settings.setArrayIndex(i);
		load_nogroup(list.at(i).value<ApiObject *>(), settings);
	}

	settings.endArray();
}

void ApiObject::load_nogroup(ApiObject *obj, QSettings& settings)
{
	auto meta = obj->metaObject();
	for (int i = meta->propertyOffset();
			i < meta->propertyCount(); i++) {
		auto prop = meta->property(i);
		if (!prop.isStored() || !prop.isReadable())
			continue;

		auto data = prop.read(obj);

		if (prop.isWritable()) {
			if (data.canConvert<QList<bool>>()) {
				auto list = load<bool>(settings, prop.name());
				if (!list.empty())
					prop.write(obj, QVariant::fromValue(list));
			} else if (data.canConvert<QList<int>>()) {
				auto list = load<int>(settings, prop.name());
				if (!list.empty())
					prop.write(obj, QVariant::fromValue(list));
			} else if (data.canConvert<QList<double>>()) {
				auto list = load<double>(settings, prop.name());
				if (!list.empty())
					prop.write(obj, QVariant::fromValue(list));
			} else if (data.canConvert<QList<QString>>()) {
				auto list = load<QString>(settings, prop.name());
				if (!list.empty())
					prop.write(obj, QVariant::fromValue(list));
			} else {
				auto value = settings.value(prop.name());

				qDebug(CAT_APIOBJECT) << "Loading property"
					<< prop.name()
					<< "value" << value;

				if (!value.isNull())
					prop.write(obj, value);
			}
		} else {
			if (data.canConvert<ApiObject *>()) {
				settings.beginGroup(prop.name());
				load_nogroup(data.value<ApiObject *>(), settings);
				settings.endGroup();
			} else if (data.canConvert<QVariantList>()) {
				load(settings, prop.name(),
						data.value<QVariantList>());
			}
		}
	}
}

void ApiObject::save_nogroup(ApiObject *obj, QSettings& settings)
{
	auto meta = obj->metaObject();

	for(int i=0;i<meta->propertyCount();i++) {
		qDebug()<<i<<" "<<meta->property(i).name();
	}

	for (int i = meta->propertyOffset();
			i < meta->propertyCount(); i++) {
		auto prop = meta->property(i);

		if (!prop.isStored() || !prop.isReadable())
			continue;

		auto data = prop.read(obj);

		if (prop.isWritable()) {
			if (data.canConvert<QList<bool>>()) {
				save<bool>(settings, prop.name(),
						data.value<QList<bool>>());
			} else if (data.canConvert<QList<int>>()) {
				save<int>(settings, prop.name(),
						data.value<QList<int>>());
			} else if (data.canConvert<QList<double>>()) {
				save<double>(settings, prop.name(),
						data.value<QList<double>>());
			} else if (data.canConvert<QList<QString>>()) {
				save<QString>(settings, prop.name(),
						data.value<QList<QString>>());
			} else {
				qDebug(CAT_APIOBJECT) << "Saving property"
					<< prop.name()
					<< "value" << data;

				settings.setValue(prop.name(), data);
			}
		} else {
			if (data.canConvert<ApiObject *>()) {
				settings.beginGroup(prop.name());
				save_nogroup(data.value<ApiObject *>(),
						settings);
				settings.endGroup();
			} else if (data.canConvert<QVariantList>()) {
				save(settings, prop.name(),
						data.value<QVariantList>());
			}
		}
	}
}

void ApiObject::load(QSettings& settings)
{
	qInfo(CAT_APIOBJECT)<<"Loading "<<objectName()<<"from: "<<settings.fileName();
	settings.beginGroup(objectName());

	load_nogroup(this, settings);

	settings.endGroup();

	Q_EMIT loadingFinished();
}

void ApiObject::save(QSettings& settings)
{
	qInfo(CAT_APIOBJECT)<<"Saving "<<objectName()<<"to: "<<settings.fileName();
	settings.beginGroup(objectName());

	save_nogroup(this, settings);

	settings.endGroup();
}
#include "moc_apiobject.cpp"
