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

#include "core/icmdexecutor.h"
#include "component/connection.h"

#include <QObject>
#include <QString>
#include <QUuid>
#include "component/component_export.h"

namespace scopy::component {

// Identity component: the root of one view's tree, parented to its Connection. The whole
// subtree is composed under it via Qt parent-child, so deleting the Context cascades to every
// child. The executor is the connection's, injected by the Controller.
//
// Concrete backends subclass this (IIOContext, ...) to carry backend-specific identity fields.
class COMPONENT_EXPORT Context : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uri READ uri CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString description READ description CONSTANT)
public:
	explicit Context(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~Context() override = default;

	QString uri() const { return m_uri; }
	QString name() const { return m_name; }
	QString description() const { return m_description; }

	void setUri(const QString &uri) { m_uri = uri; }
	void setName(const QString &name)
	{
		m_name = name;
		setObjectName(name);
	}
	void setDescription(const QString &description) { m_description = description; }

	// The connection this view is built on (= parent()), or nullptr before it is attached.
	Connection *connection() const { return qobject_cast<Connection *>(parent()); }
	BackendId backend() const { return m_backend; }
	void setBackend(const BackendId &id) { m_backend = id; }

	// The executor driving all I/O in this context's subtree; the connection's, not owned.
	ICmdExecutor *executor() const { return m_executor; }
	void setExecutor(ICmdExecutor *exec) { m_executor = exec; }

	// Cancellation, delegated to this context's executor. cancelCommandById cancels
	// one queued command by its QUuid (the id returned by readAsync/writeAsync);
	// cancelAllCommands cancels every command still pending in this context.
	// Cooperative and pre-execution only — a command already running on a worker
	// cannot be stopped.
	void cancelCommandById(const QUuid &id)
	{
		if(m_executor) {
			m_executor->cancelById(id);
		}
	}
	void cancelAllCommands()
	{
		if(m_executor) {
			m_executor->cancelAll();
		}
	}

protected:
	QString m_uri;
	QString m_name;
	QString m_description;
	BackendId m_backend;
	ICmdExecutor *m_executor = nullptr;
};

} // namespace scopy::component
