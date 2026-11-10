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
#include "component/connection.h"
#include "component/context.h"
#include "core/calltrace.h"

#include <QMetaObject>

#include <atomic>
#include <mutex>

using namespace scopy;
using namespace scopy::component;

namespace {
std::mutex s_mutex;
std::atomic<CommandMonitor *> s_instance{nullptr}; // read by deliver() on executor threads

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
} // namespace

CommandMonitor::CommandMonitor(QObject *parent)
	: QObject(parent)
{
	qRegisterMetaType<scopy::CallArg>("scopy::CallArg");
	qRegisterMetaType<scopy::CallRecord>("scopy::CallRecord");
	qRegisterMetaType<scopy::CommandLogEntry>("scopy::CommandLogEntry");
	CallTrace::setSink(&CommandMonitor::deliver);
}

CommandMonitor *CommandMonitor::GetInstance()
{
	std::lock_guard<std::mutex> lock(s_mutex);
	if(!s_instance.load()) {
		s_instance.store(new CommandMonitor());
	}
	return s_instance.load();
}

void CommandMonitor::deliver(const CommandLogEntry &entry)
{
	CommandMonitor *m = s_instance.load(std::memory_order_acquire);
	if(m) {
		Q_EMIT m->commandFinished(entry);
	}
}

bool CommandMonitor::isEnabled() const { return CallTrace::isEnabled(); }

void CommandMonitor::setEnabled(bool enabled)
{
	bool expected = !enabled;
	if(CallTrace::s_enabled.compare_exchange_strong(expected, enabled)) {
		Q_EMIT enabledChanged(enabled);
	}
}

bool CommandMonitor::captureData() const { return CallTrace::captureData(); }

void CommandMonitor::setCaptureData(bool capture)
{
	if(CallTrace::captureData() == capture) {
		return;
	}
	CallTrace::setCaptureData(capture);
	Q_EMIT captureDataChanged(capture);
}

int CommandMonitor::previewBytes() const { return CallTrace::previewBytes(); }

void CommandMonitor::setPreviewBytes(int bytes) { CallTrace::setPreviewBytes(bytes); }

CommandOrigin CommandMonitor::originOf(const QObject *node)
{
	CommandOrigin o;
	if(!node) {
		return o;
	}
	o.object = quintptr(node);
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
	if(!cmd || !CallTrace::isEnabled()) {
		return;
	}
	cmd->setOrigin(originOf(leaf));
}
