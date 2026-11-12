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

#include "readpolicy.h"

#include <QObject>
#include <QSet>

#include <component/attribute.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
// Attributes already read on selection; entries leave with their attribute.
QSet<const QObject *> &autoRead()
{
	static QSet<const QObject *> set;
	return set;
}

int g_suppressed = 0;
} // namespace

void ReadPolicy::initPreferences() { Preferences::init(PREF, QString::fromLatin1(ONLY_UNCACHED)); }

QStringList ReadPolicy::options() { return {QString::fromLatin1(ONLY_UNCACHED), QString::fromLatin1(EVERY_SELECTION)}; }

bool ReadPolicy::everySelection()
{
	return Preferences::get(QString::fromLatin1(PREF)).toString() == QLatin1String(EVERY_SELECTION);
}

bool ReadPolicy::takeAutoRead(component::Attribute *attr)
{
	if(!attr || !attr->readCapability() || suppressed()) {
		return false;
	}
	if(everySelection()) {
		return true;
	}
	if(!attr->cachedValue().isEmpty() || autoRead().contains(attr)) {
		return false;
	}
	autoRead().insert(attr);
	QObject::connect(attr, &QObject::destroyed, attr, [](QObject *o) { autoRead().remove(o); });
	return true;
}

ReadPolicy::Suppress::Suppress() { ++g_suppressed; }

ReadPolicy::Suppress::~Suppress() { --g_suppressed; }

bool ReadPolicy::suppressed() { return g_suppressed > 0; }
