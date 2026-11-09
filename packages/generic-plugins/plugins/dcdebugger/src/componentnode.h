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

#ifndef COMPONENTNODE_H
#define COMPONENTNODE_H

#include "scopy-dcdebugger_export.h"

#include <QObject>
#include <QString>

namespace scopy::component {
class Attribute;
}

namespace scopy::dcdebugger {

// Base component type of a tree node, decided from the component bases / metaobject only.
enum class NodeKind
{
	Group,
	Connection,
	Context,
	Device,
	Channel,
	Attribute,
	InputStream,
	OutputStream,
	ScanElement,
	Trigger,
	Triggerable,
	RegisterReader,
	RegisterWriter,
	BusTransfer,
	DongleManager,
	Ping,
	SequenceRunner,
	Other, // a component leaf with no generic base (shown by class name)
	Hidden // plumbing (readers/writers of an attribute, codecs, executors): not shown
};

class SCOPY_DCDEBUGGER_EXPORT ComponentNode
{
public:
	static NodeKind kindOf(const QObject *obj);
	// Base type name shown in the info column ("Device", "InputStream", ...).
	static QString kindName(NodeKind kind);
	// Group the node is listed under in its parent ("Devices", "Channels", ...).
	static QString groupOf(const QObject *obj, NodeKind kind);
	// Sort key of a group inside its parent.
	static int groupOrder(const QString &group);
	static QString className(const QObject *obj);

	static QString name(const QObject *obj, NodeKind kind);
	static QString value(const QObject *obj, NodeKind kind);
	static QString info(const QObject *obj, NodeKind kind);
	// The tags::Direction attribute of a channel, if it has one.
	static component::Attribute *directionAttribute(const QObject *channel);
};

} // namespace scopy::dcdebugger

#endif // COMPONENTNODE_H
