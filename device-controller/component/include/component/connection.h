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

#include <QList>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QString>
#include "component/component_export.h"

namespace scopy::component {

class CommandMonitor;
class Context;
class Ping;

using BackendId = QString;
using ConnectionKind = QString;

// One physical link to a device (one uri), owned by the Controller. Holds the executor shared by
// every view built on it and one Context per compatible backend. Subclasses carry the native
// handle (IIOConnection). Owns the command monitor (call trace) of the link.
class COMPONENT_EXPORT Connection : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uri READ uri CONSTANT)
public:
	explicit Connection(QString uri, ConnectionKind kind, QObject *parent = nullptr);
	~Connection() override;

	QString uri() const { return m_uri; }
	ConnectionKind kind() const { return m_kind; }

	ICmdExecutor *executor() const { return m_executor; }
	// Also installs the monitor's trace config on the executor.
	void setExecutor(ICmdExecutor *executor);

	// Command call trace of this connection (one per connection, child of it, off by default).
	CommandMonitor *monitor() const;

	// Compatible backends, decided once by the Controller right after open.
	QList<BackendId> backends() const { return m_backends; }
	void setBackends(const QList<BackendId> &backends) { m_backends = backends; }

	// Built views, in creation order.
	QList<Context *> contexts() const;
	QList<BackendId> builtBackends() const;
	Context *context(const BackendId &id) const;
	BackendId backendOf(const Context *ctx) const;

	template <typename T>
	T *as()
	{
		return qobject_cast<T *>(this);
	}
	template <typename T>
	const T *as() const
	{
		return qobject_cast<const T *>(this);
	}

	// The link's reachability check, shared by every view; nullptr until start() or when the
	// provider has none. Views pause it around long transfers instead of running their own.
	Ping *ping() const { return m_ping; }

	// Called by the Controller once the executor is set (e.g. start pinging).
	virtual void start() {}

	// Controller only. add() reparents ctx to this connection and injects `executor` (the view's
	// scope; the connection's executor when null).
	void add(const BackendId &id, Context *ctx, ICmdExecutor *executor = nullptr);
	Context *take(const BackendId &id);

Q_SIGNALS:
	void lost();
	void contextCreated(scopy::component::Context *ctx);
	void contextDestroyed(scopy::component::Context *ctx);

protected:
	QString m_uri;
	ConnectionKind m_kind;
	ICmdExecutor *m_executor = nullptr;
	QPointer<CommandMonitor> m_monitor; // QPointer: subclasses may delete children early
	QList<BackendId> m_backends;
	QList<QPair<BackendId, Context *>> m_contexts;
	Ping *m_ping = nullptr;
};

} // namespace scopy::component
