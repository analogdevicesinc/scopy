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

#include "componentnode.h"

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>
#include <component/channel.h>
#include <component/commandmonitor.h>
#include <component/connection.h>
#include <component/context.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <component/outputstream.h>
#include <component/ping.h>
#include <component/registerreader.h>
#include <component/registerwriter.h>
#include <component/samplecodec.h>
#include <component/scanelement.h>
#include <component/tags.h>
#include <component/trigger.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const QString GROUP_VIEWS = QStringLiteral("Views");
const QString GROUP_ATTRIBUTES = QStringLiteral("Attributes");
const QString GROUP_DEVICES = QStringLiteral("Devices");
const QString GROUP_CHANNELS = QStringLiteral("Channels");
const QString GROUP_STREAMS = QStringLiteral("Streams");
const QString GROUP_CAPABILITIES = QStringLiteral("Capabilities");
const QString GROUP_SCAN_ELEMENTS = QStringLiteral("Scan elements");

QString shortName(const char *className)
{
	const QString full = QString::fromLatin1(className);
	const int pos = full.lastIndexOf(QStringLiteral("::"));
	return pos < 0 ? full : full.mid(pos + 2);
}
} // namespace

NodeKind ComponentNode::kindOf(const QObject *obj)
{
	if(!obj) {
		return NodeKind::Hidden;
	}
	if(qobject_cast<const Connection *>(obj)) {
		return NodeKind::Connection;
	}
	if(qobject_cast<const Context *>(obj)) {
		return NodeKind::Context;
	}
	if(qobject_cast<const Device *>(obj)) {
		return NodeKind::Device;
	}
	if(qobject_cast<const Channel *>(obj)) {
		return NodeKind::Channel;
	}
	if(qobject_cast<const Attribute *>(obj)) {
		return NodeKind::Attribute;
	}
	if(qobject_cast<const InputStream *>(obj)) {
		return NodeKind::InputStream;
	}
	if(qobject_cast<const OutputStream *>(obj)) {
		return NodeKind::OutputStream;
	}
	if(qobject_cast<const ScanElement *>(obj)) {
		return NodeKind::ScanElement;
	}
	if(qobject_cast<const Trigger *>(obj)) {
		return NodeKind::Trigger;
	}
	if(qobject_cast<const Triggerable *>(obj)) {
		return NodeKind::Triggerable;
	}
	if(qobject_cast<const RegisterReader *>(obj)) {
		return NodeKind::RegisterReader;
	}
	if(qobject_cast<const RegisterWriter *>(obj)) {
		return NodeKind::RegisterWriter;
	}
	if(qobject_cast<const Ping *>(obj)) {
		return NodeKind::Ping;
	}
	if(qobject_cast<const AttributeReader *>(obj) || qobject_cast<const AttributeWriter *>(obj) ||
	   qobject_cast<const SampleCodec *>(obj) || qobject_cast<const CommandMonitor *>(obj) ||
	   obj->inherits("scopy::PooledCmdExecutor") || obj->inherits("QTimer")) {
		return NodeKind::Hidden;
	}
	return NodeKind::Other;
}

QString ComponentNode::kindName(NodeKind kind)
{
	switch(kind) {
	case NodeKind::Group:
		return QStringLiteral("Group");
	case NodeKind::Connection:
		return QStringLiteral("Connection");
	case NodeKind::Context:
		return QStringLiteral("Context");
	case NodeKind::Device:
		return QStringLiteral("Device");
	case NodeKind::Channel:
		return QStringLiteral("Channel");
	case NodeKind::Attribute:
		return QStringLiteral("Attribute");
	case NodeKind::InputStream:
		return QStringLiteral("InputStream");
	case NodeKind::OutputStream:
		return QStringLiteral("OutputStream");
	case NodeKind::ScanElement:
		return QStringLiteral("ScanElement");
	case NodeKind::Trigger:
		return QStringLiteral("Trigger");
	case NodeKind::Triggerable:
		return QStringLiteral("Triggerable");
	case NodeKind::RegisterReader:
		return QStringLiteral("RegisterReader");
	case NodeKind::RegisterWriter:
		return QStringLiteral("RegisterWriter");
	case NodeKind::Ping:
		return QStringLiteral("Ping");
	case NodeKind::Other:
		return QStringLiteral("Component");
	case NodeKind::Hidden:
		break;
	}
	return {};
}

QString ComponentNode::groupOf(const QObject *obj, NodeKind kind)
{
	switch(kind) {
	case NodeKind::Context:
		return GROUP_VIEWS;
	case NodeKind::Device:
		return GROUP_DEVICES;
	case NodeKind::Channel:
		return GROUP_CHANNELS;
	case NodeKind::Attribute:
		return GROUP_ATTRIBUTES;
	case NodeKind::InputStream:
	case NodeKind::OutputStream:
		return GROUP_STREAMS;
	case NodeKind::ScanElement:
		return GROUP_SCAN_ELEMENTS;
	case NodeKind::Trigger:
	case NodeKind::Triggerable:
	case NodeKind::RegisterReader:
	case NodeKind::RegisterWriter:
	case NodeKind::Ping:
		return GROUP_CAPABILITIES;
	case NodeKind::Other:
		// Same-class leaves with no generic base are grouped by their class.
		return className(obj);
	default:
		break;
	}
	return {};
}

int ComponentNode::groupOrder(const QString &group)
{
	static const QStringList order = {GROUP_VIEWS,	 GROUP_ATTRIBUTES,    GROUP_DEVICES,	 GROUP_CHANNELS,
					  GROUP_STREAMS, GROUP_SCAN_ELEMENTS, GROUP_CAPABILITIES};
	const int idx = order.indexOf(group);
	return idx < 0 ? order.size() : idx;
}

QString ComponentNode::className(const QObject *obj)
{
	return obj ? shortName(obj->metaObject()->className()) : QString();
}

QString ComponentNode::name(const QObject *obj, NodeKind kind)
{
	switch(kind) {
	case NodeKind::Connection:
		return static_cast<const Connection *>(obj)->uri();
	case NodeKind::Context: {
		auto *ctx = static_cast<const Context *>(obj);
		return ctx->backend().isEmpty() ? ctx->name() : ctx->backend();
	}
	case NodeKind::Device: {
		auto *dev = static_cast<const Device *>(obj);
		return dev->name().isEmpty() ? dev->id() : dev->name();
	}
	case NodeKind::Channel:
		return static_cast<const Channel *>(obj)->id();
	case NodeKind::Attribute:
		return static_cast<const Attribute *>(obj)->name();
	case NodeKind::InputStream:
		return QStringLiteral("input buffer %1").arg(static_cast<const Stream *>(obj)->bufferIndex());
	case NodeKind::OutputStream:
		return QStringLiteral("output buffer %1").arg(static_cast<const Stream *>(obj)->bufferIndex());
	case NodeKind::ScanElement:
		return static_cast<const ScanElement *>(obj)->id();
	case NodeKind::Trigger:
		return static_cast<const Trigger *>(obj)->name();
	default:
		break;
	}
	return obj->objectName().isEmpty() ? className(obj) : obj->objectName();
}

QString ComponentNode::value(const QObject *obj, NodeKind kind)
{
	switch(kind) {
	case NodeKind::Connection:
		return static_cast<const Connection *>(obj)->kind();
	case NodeKind::Context: {
		auto *ctx = static_cast<const Context *>(obj);
		return ctx->backend().isEmpty() ? QString() : ctx->name();
	}
	case NodeKind::Device: {
		auto *dev = static_cast<const Device *>(obj);
		return dev->label().isEmpty() ? dev->id() : dev->label();
	}
	case NodeKind::Channel: {
		auto *ch = static_cast<const Channel *>(obj);
		return ch->label().isEmpty() ? ch->name() : ch->label();
	}
	case NodeKind::Attribute: {
		auto *attr = static_cast<const Attribute *>(obj);
		if(attr->cachedValue().isEmpty() || attr->unit().isEmpty()) {
			return attr->cachedValue();
		}
		return attr->cachedValue() + " " + attr->unit();
	}
	case NodeKind::InputStream:
	case NodeKind::OutputStream:
		return static_cast<const Stream *>(obj)->isOpen() ? QStringLiteral("open") : QStringLiteral("closed");
	case NodeKind::RegisterReader:
	case NodeKind::RegisterWriter:
		return QStringLiteral("addr %1 bit / value %2 bit")
			.arg(obj->property("addressBits").toInt())
			.arg(obj->property("valueBits").toInt());
	case NodeKind::ScanElement:
		return static_cast<const ScanElement *>(obj)->isEnabled() ? QStringLiteral("enabled")
									  : QStringLiteral("disabled");
	default:
		break;
	}
	return {};
}

Attribute *ComponentNode::directionAttribute(const QObject *channel)
{
	const QList<Attribute *> tagged = findByTag<Attribute>(channel, tags::Direction, false);
	return tagged.isEmpty() ? nullptr : tagged.first();
}

QString ComponentNode::info(const QObject *obj, NodeKind kind)
{
	switch(kind) {
	case NodeKind::Channel: {
		// A bidirectional line reports its live direction; otherwise isOutput() is the identity.
		if(const Attribute *dir = directionAttribute(obj)) {
			const QString v = dir->cachedValue();
			return kindName(kind) + (v.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(v));
		}
		return kindName(kind) +
			(static_cast<const Channel *>(obj)->isOutput() ? QStringLiteral(" (output)")
								       : QStringLiteral(" (input)"));
	}
	case NodeKind::Attribute:
		return kindName(kind) +
			(static_cast<const Attribute *>(obj)->isWritable() ? QStringLiteral(" (rw)")
									   : QStringLiteral(" (ro)"));
	case NodeKind::Other:
		return className(obj);
	default:
		break;
	}
	return kindName(kind);
}
