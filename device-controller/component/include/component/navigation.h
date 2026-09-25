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

#pragma once

#include "component/attribute.h"
#include "component/channel.h"

#include <QObject>
#include <QString>

// Shared navigation helpers over the component tree. Plain by-name / by-type
// lookups are already one-liners via QObject::findChild<T*>("name",
// Qt::FindDirectChildrenOnly), so those need no wrapper. This header collects the
// navigation primitives that findChild cannot express directly; add new shared
// finders here as further plugins migrate. (See also streamsOf<T>/streamAt<T> in
// component/stream.h for the streaming equivalents.)

namespace scopy::component {

// Direct-child Channel matching both id and direction. Needed because an input
// and an output channel can share the same id (e.g. "altvoltage0"), and only a
// Channel's id is exposed as its objectName, so findChild<Channel*>(id) alone is
// ambiguous. Returns nullptr when no match exists.
inline Channel *channelById(const QObject *device, const QString &id, bool isOutput)
{
	if(!device) {
		return nullptr;
	}
	for(Channel *c : device->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(c->id() == id && c->isOutput() == isOutput) {
			return c;
		}
	}
	return nullptr;
}

// Direct-child Attribute by name on any tree node (device, channel or stream).
// A thin, self-documenting wrapper over the findChild<Attribute*> one-liner used
// throughout builder recipes; returns nullptr when the node has no such attribute.
inline Attribute *attributeByName(const QObject *node, const QString &name)
{
	if(!node) {
		return nullptr;
	}
	return node->findChild<Attribute *>(name, Qt::FindDirectChildrenOnly);
}

} // namespace scopy::component
