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

#include "component/commandmonitor.h"

#include "component/attribute.h"
#include "component/channel.h"
#include "component/connection.h"
#include "component/context.h"
#include "component/device.h"
#include "component/inputstream.h"
#include "component/outputstream.h"
#include "component/scanelement.h"
#include "component/tags.h"
#include "core/calltrace.h"

#include <QMetaObject>

using namespace scopy;
using namespace scopy::component;

namespace {
// Name of the component base a leaf derives from ("InputStream", "RegisterReader", ...).
QString componentBaseName(const QObject *obj)
{
	const QMetaObject *mo = obj->metaObject();
	while(mo->superClass() && mo->superClass() != &QObject::staticMetaObject) {
		mo = mo->superClass();
	}
	QString name = QString::fromLatin1(mo->className());
	const qsizetype sep = name.lastIndexOf(QLatin1String("::"));
	return sep >= 0 ? name.mid(sep + 2) : name;
}

void setOnce(QVariantMap &scope, const char *key, const QVariant &value)
{
	const QString k = QString::fromLatin1(key);
	if(!scope.contains(k)) {
		scope.insert(k, value);
	}
}

// Identifying names of the nodes from `node` up to its Context (the nearest one of each kind).
QVariantMap scopeOf(const QObject *node)
{
	namespace os = originscope;
	QVariantMap scope;
	scope.insert(QString::fromLatin1(os::Kind), componentBaseName(node));
	for(const QObject *cur = node; cur; cur = cur->parent()) {
		if(qobject_cast<const Connection *>(cur)) {
			break;
		}
		if(auto *a = qobject_cast<const Attribute *>(cur)) {
			if(!scope.contains(QString::fromLatin1(os::Attribute))) {
				scope.insert(QString::fromLatin1(os::Attribute), a->objectName());
				const QObject *p = a->parent();
				const char *of = qobject_cast<const Channel *>(p) ? "channel"
					: qobject_cast<const Device *>(p)	  ? "device"
					: qobject_cast<const Context *>(p)	  ? "context"
					: qobject_cast<const Stream *>(p)	  ? "buffer"
										  : "";
				if(*of) {
					scope.insert(QString::fromLatin1(os::AttributeOf), QString::fromLatin1(of));
				}
				if(hasTag(a, QString::fromLatin1(tags::DebugAttribute))) {
					scope.insert(QString::fromLatin1(os::Debug), true);
				}
			}
		} else if(auto *ch = qobject_cast<const Channel *>(cur)) {
			setOnce(scope, os::Channel, ch->id());
			setOnce(scope, os::Output, ch->isOutput());
		} else if(auto *el = qobject_cast<const ScanElement *>(cur)) {
			setOnce(scope, os::Channel, el->id());
			setOnce(scope, os::Output, el->isOutput());
		} else if(auto *st = qobject_cast<const Stream *>(cur)) {
			const bool out = qobject_cast<const OutputStream *>(st) != nullptr;
			setOnce(scope, os::Stream, QString::fromLatin1(out ? "output" : "input"));
			setOnce(scope, os::BufferIndex, st->bufferIndex());
			setOnce(scope, os::Output, out);
		} else if(auto *d = qobject_cast<const Device *>(cur)) {
			setOnce(scope, os::Device, d->name().isEmpty() ? d->id() : d->name());
			setOnce(scope, os::DeviceId, d->id());
		} else if(auto *c = qobject_cast<const Context *>(cur)) {
			setOnce(scope, os::Context, c->name());
			break;
		}
	}
	return scope;
}
} // namespace

CommandMonitor::CommandMonitor(Connection *connection)
	: QObject(connection)
	, m_uri(connection ? connection->uri() : QString())
	, m_config(std::make_shared<TraceConfig>())
{
	qRegisterMetaType<scopy::CallArg>("scopy::CallArg");
	qRegisterMetaType<scopy::CallRecord>("scopy::CallRecord");
	qRegisterMetaType<scopy::CommandLogEntry>("scopy::CommandLogEntry");
	m_config->setSink([this](const CommandLogEntry &entry) { deliver(entry); });
}

CommandMonitor::~CommandMonitor()
{
	// The executor may outlive this object (it holds the config): stop recording and wait for a
	// delivery in progress on an executor thread; nothing reaches this object afterwards.
	m_config->setEnabled(false);
	m_config->setSink(nullptr);
}

CommandMonitor *CommandMonitor::of(const QObject *node)
{
	for(const QObject *cur = node; cur; cur = cur->parent()) {
		if(auto *conn = qobject_cast<const Connection *>(cur)) {
			return conn->monitor();
		}
	}
	return nullptr;
}

Connection *CommandMonitor::connection() const { return qobject_cast<Connection *>(parent()); }

void CommandMonitor::deliver(CommandLogEntry entry)
{
	entry.uri = m_uri;
	Q_EMIT commandFinished(entry);
}

bool CommandMonitor::isEnabled() const { return m_config->isEnabled(); }

void CommandMonitor::setEnabled(bool enabled)
{
	if(m_config->setEnabled(enabled)) {
		Q_EMIT enabledChanged(enabled);
	}
}

bool CommandMonitor::captureData() const { return m_config->captureData(); }

void CommandMonitor::setCaptureData(bool capture)
{
	if(m_config->setCaptureData(capture)) {
		Q_EMIT captureDataChanged(capture);
	}
}

int CommandMonitor::previewBytes() const { return m_config->previewBytes(); }

void CommandMonitor::setPreviewBytes(int bytes) { m_config->setPreviewBytes(bytes); }

CommandOrigin CommandMonitor::originOf(const QObject *node)
{
	CommandOrigin o;
	if(!node) {
		return o;
	}
	o.object = quintptr(node);
	o.scope = scopeOf(node);
	QStringList segments;
	if(node->objectName().isEmpty() && !qobject_cast<const Attribute *>(node->parent())) {
		segments.prepend(componentBaseName(node));
	}
	for(const QObject *cur = node; cur; cur = cur->parent()) {
		if(auto *conn = qobject_cast<const Connection *>(cur)) {
			o.uri = conn->uri();
			break;
		}
		if(!cur->objectName().isEmpty()) {
			segments.prepend(cur->objectName());
		}
		if(auto *ctx = qobject_cast<const Context *>(cur)) {
			o.backendId = ctx->backend();
			o.uri = ctx->uri();
		}
	}
	o.path = segments.join(QLatin1Char('/'));
	return o;
}

void CommandMonitor::tag(Command *cmd, const QObject *leaf)
{
	if(!cmd || !TraceConfig::anyEnabled()) {
		return;
	}
	cmd->setOrigin(originOf(leaf));
}
