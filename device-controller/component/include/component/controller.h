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

#include "component/connection.h"
#include "component/connectionprovider.h"
#include "component/context.h"
#include "component/contextbackend.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <memory>
#include <mutex>
#include "component/component_export.h"

namespace scopy::component {

class DeviceMonitor;

// Refcounted access to an open Connection (no view).
class COMPONENT_EXPORT ConnectionHandle
{
public:
	ConnectionHandle() = default;
	ConnectionHandle(QString uri, Connection *conn)
		: m_uri(std::move(uri))
		, m_conn(conn)
	{}
	~ConnectionHandle() { reset(); }

	ConnectionHandle(const ConnectionHandle &) = delete;
	ConnectionHandle &operator=(const ConnectionHandle &) = delete;

	ConnectionHandle(ConnectionHandle &&o) noexcept
		: m_uri(std::move(o.m_uri))
		, m_conn(std::exchange(o.m_conn, nullptr))
	{}
	ConnectionHandle &operator=(ConnectionHandle &&o) noexcept
	{
		if(this != &o) {
			reset();
			m_uri = std::move(o.m_uri);
			m_conn = std::exchange(o.m_conn, nullptr);
		}
		return *this;
	}

	Connection *get() const { return m_conn; }
	Connection *operator->() const { return m_conn; }
	explicit operator bool() const { return m_conn != nullptr; }

	void reset();

private:
	QString m_uri;
	Connection *m_conn = nullptr;
};

// Refcounted access to one view (Context) of a connection.
class COMPONENT_EXPORT ContextHandle
{
public:
	ContextHandle() = default;
	ContextHandle(QString uri, BackendId backend, Context *ctx)
		: m_uri(std::move(uri))
		, m_backend(std::move(backend))
		, m_ctx(ctx)
	{}
	~ContextHandle() { reset(); }

	ContextHandle(const ContextHandle &) = delete;
	ContextHandle &operator=(const ContextHandle &) = delete;

	ContextHandle(ContextHandle &&o) noexcept
		: m_uri(std::move(o.m_uri))
		, m_backend(std::move(o.m_backend))
		, m_ctx(std::exchange(o.m_ctx, nullptr))
	{}
	ContextHandle &operator=(ContextHandle &&o) noexcept
	{
		if(this != &o) {
			reset();
			m_uri = std::move(o.m_uri);
			m_backend = std::move(o.m_backend);
			m_ctx = std::exchange(o.m_ctx, nullptr);
		}
		return *this;
	}

	Context *get() const { return m_ctx; }
	Context *operator->() const { return m_ctx; }
	Context &operator*() const { return *m_ctx; }
	explicit operator bool() const { return m_ctx != nullptr; }
	BackendId backend() const { return m_backend; }
	Connection *connection() const { return m_ctx ? m_ctx->connection() : nullptr; }

	void reset();

private:
	QString m_uri;
	BackendId m_backend;
	Context *m_ctx = nullptr;
};

class COMPONENT_EXPORT Controller : public QObject
{
	Q_OBJECT
protected:
	explicit Controller(QObject *parent = nullptr);
	~Controller() override;

public:
	Controller(const Controller &) = delete;
	void operator=(const Controller &) = delete;

	static Controller *GetInstance();

	// Registries. libiio (provider "iio" + view "iio") is registered at construction; packages
	// can add providers and backends at runtime (e.g. from Plugin::init()). Thread-safe.
	bool registerProvider(std::shared_ptr<ConnectionProvider> provider);
	QList<ConnectionKind> providers() const;
	std::shared_ptr<ConnectionProvider> provider(const ConnectionKind &kind) const;

	bool registerBackend(std::shared_ptr<ContextBackend> backend);
	// Fails while a view of that backend exists.
	bool unregisterBackend(const BackendId &id);
	QList<BackendId> backends() const;
	std::shared_ptr<ContextBackend> backend(const BackendId &id) const;

	// Discovery through one provider. scan() blocks; call it off the UI thread.
	QVector<ScanResult> scan(const ConnectionKind &kind, const QString &filter = QString()) const;
	DeviceMonitor *monitor(const ConnectionKind &kind) const;

	// Opens (or shares) the connection at uri. An empty kind means libiio. Right after a new
	// connection opens, every registered backend of that kind is asked compatible(); the result
	// is Connection::backends(). The executor gets min(maxThreads, backends' maxThreads()).
	static ConnectionHandle open(const QString &uri, const ConnectionKind &kind = {}, int maxThreads = 1)
	{
		return GetInstance()->_open(uri, kind, maxThreads);
	}

	// The `backend` view of uri, built on first use; opens the connection if needed. An empty
	// backend means "iio". Fails if the backend is not compatible with the connection.
	static ContextHandle connectCtx(const QString &uri, const BackendId &backend = {}, int maxThreads = 1)
	{
		return GetInstance()->_connectCtx(uri, backend, maxThreads);
	}

	// An existing view only; never opens or builds anything.
	static ContextHandle context(const QString &uri, const BackendId &backend = {})
	{
		return GetInstance()->_acquireExisting(uri, backend);
	}

	// Adopt an externally-built Context as the only view of a new connection (tests).
	ContextHandle adopt(const QString &uri, Context *ctx, const BackendId &backend = "adopted");

Q_SIGNALS:
	void connectionOpened(scopy::component::Connection *conn);
	void connectionClosed(scopy::component::Connection *conn);
	void connectionLost(scopy::component::Connection *conn);
	// Connection::backends() changed after a late registerBackend()/unregisterBackend().
	void backendsChanged(scopy::component::Connection *conn);
	void contextCreated(scopy::component::Connection *conn, scopy::component::Context *ctx);
	void contextDestroyed(scopy::component::Connection *conn, scopy::component::Context *ctx);
	void providerRegistered(const QString &kind);
	void backendRegistered(const QString &id);
	void backendUnregistered(const QString &id);

private:
	friend class ConnectionHandle;
	friend class ContextHandle;

	struct ConnEntry
	{
		Connection *conn = nullptr;
		int connRefs = 0;
		int threads = 1;
		QMap<BackendId, int> viewRefs;
	};
	using ConnMap = QMap<QString, ConnEntry>;

	ConnectionHandle _open(const QString &uri, const ConnectionKind &kind, int maxThreads);
	ContextHandle _connectCtx(const QString &uri, const BackendId &backend, int maxThreads);
	ContextHandle _acquireExisting(const QString &uri, const BackendId &backend);
	void _releaseConnection(const QString &uri);
	void _releaseView(const QString &uri, const BackendId &backend);

	// All under m_ctxMutex.
	ConnMap::iterator _openLocked(const QString &uri, const ConnectionKind &kind, int maxThreads);
	void _closeIfUnused(ConnMap::iterator it);
	QList<std::shared_ptr<ContextBackend>> _backendsOfKind(const ConnectionKind &kind) const;
	std::shared_ptr<ContextBackend> _resolveBackend(const BackendId &backend) const;

	static Controller *pinstance_;
	static std::mutex mutex_;
	// Lock order: m_ctxMutex, then m_registryMutex.
	std::mutex m_ctxMutex;
	ConnMap m_connections;
	mutable std::mutex m_registryMutex;
	QMap<ConnectionKind, std::shared_ptr<ConnectionProvider>> m_providers;
	QMap<BackendId, std::shared_ptr<ContextBackend>> m_backends;
};

} // namespace scopy::component
