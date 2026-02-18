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

#include "iiowidgetgroup.h"
#include <QLoggingCategory>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_IIOWIDGETGROUP, "IIOWidgetGroup")

using namespace scopy;

IIOWidgetGroup::IIOWidgetGroup(QObject *parent)
	: QObject(parent)
{}

IIOWidgetGroup::~IIOWidgetGroup() {}

void IIOWidgetGroup::add(IIOWidget *widget)
{
	if(!widget) {
		qWarning(CAT_IIOWIDGETGROUP) << "Cannot add a null widget";
		return;
	}

	QString key = generateKey(widget->getRecipe());
	if(m_widgets.contains(key)) {
		qWarning(CAT_IIOWIDGETGROUP) << "Key collision: widget with key" << key << "already exists";
		return;
	}

	m_widgets.insert(key, widget);
}

void IIOWidgetGroup::add(QList<IIOWidget *> widgets)
{
	for(IIOWidget *widget : widgets) {
		add(widget);
	}
}

IIOWidget *IIOWidgetGroup::get(const QString &key) const { return m_widgets.value(key, nullptr); }

QMap<QString, IIOWidget *> IIOWidgetGroup::getAll() const { return m_widgets; }

void IIOWidgetGroup::remove(const QString &key) { m_widgets.remove(key); }

bool IIOWidgetGroup::contains(const QString &key) const { return m_widgets.contains(key); }

QStringList IIOWidgetGroup::keys() const { return m_widgets.keys(); }

void IIOWidgetGroup::clear() { m_widgets.clear(); }

QString IIOWidgetGroup::generateKey(const IIOWidgetFactoryRecipe &recipe)
{
	QString deviceName;
	QString channelId;
	QString attribute = recipe.data;

	if(recipe.channel) {
		const char *name = iio_channel_get_id(recipe.channel);
		channelId = name ? QString(name) : QString();
		if(iio_channel_is_output(recipe.channel)) {
			channelId = channelId + "_out";
		} else {
			channelId = channelId + "_in";
		}

		struct iio_device *dev = const_cast<iio_device *>(iio_channel_get_device(recipe.channel));
		if(dev) {
			const char *devName = iio_device_get_name(dev);
			if(!devName) {
				devName = iio_device_get_id(dev);
			}
			deviceName = devName ? QString(devName) : QString();
		}
	} else if(recipe.device) {
		const char *devName = iio_device_get_name(recipe.device);
		if(!devName) {
			devName = iio_device_get_id(recipe.device);
		}
		deviceName = devName ? QString(devName) : QString();
		return deviceName + "/" + attribute;
	} else if(recipe.context) {
		return QStringLiteral("context/") + attribute;
	}

	return deviceName + "/" + channelId + "/" + attribute;
}

#include "moc_iiowidgetgroup.cpp"
