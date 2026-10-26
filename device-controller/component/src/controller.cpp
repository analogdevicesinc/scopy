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

#include "component/controller.h"

#include "component/backends/iio/iioctxbackend.h"
#include "component/backends/iio/iioprovider.h"
#include "core/pooledcmdexecutor.h"

#include <QLoggingCategory>

#include <algorithm>

Q_LOGGING_CATEGORY(CAT_CONTROLLER, "Controller")

using namespace scopy::component;

void ConnectionHandle::reset()
{
	if(m_conn) {
		Controller::GetInstance()->_releaseConnection(m_uri);
		m_conn = nullptr;
	}
}

void ContextHandle::reset()
{
	if(m_ctx) {
		Controller::GetInstance()->_releaseView(m_uri, m_backend);
		m_ctx = nullptr;
	}
}

Controller *Controller::pinstance_{nullptr};
std::mutex Controller::mutex_;

Controller *Controller::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(pinstance_ == nullptr) {
		pinstance_ = new Controller();
	}
	return pinstance_;
}

Controller::Controller(QObject *parent)
	: QObject(parent)
{
	registerProvider(std::make_shared<iio::IIOProvider>());
	registerBackend(std::make_shared<iio::IIOCtxBackend>());
}

Controller::~Controller() = default;

// ---- registries ----

bool Controller::registerProvider(std::shared_ptr<ConnectionProvider> provider)
{
	if(!provider || provider->kind().isEmpty()) {
		return false;
	}
	const ConnectionKind kind = provider->kind();
	{
		std::lock_guard<std::mutex> lock(m_registryMutex);
		if(m_providers.contains(kind)) {
			qWarning(CAT_CONTROLLER) << "provider already registered:" << kind;
			return false;
		}
		m_providers.insert(kind, std::move(provider));
	}
	Q_EMIT providerRegistered(kind);
	return true;
}

QList<ConnectionKind> Controller::providers() const
{
	std::lock_guard<std::mutex> lock(m_registryMutex);
	return m_providers.keys();
}

std::shared_ptr<ConnectionProvider> Controller::provider(const ConnectionKind &kind) const
{
	std::lock_guard<std::mutex> lock(m_registryMutex);
	return m_providers.value(kind.isEmpty() ? ConnectionKind(iio::IIOProvider::KIND) : kind);
}

bool Controller::registerBackend(std::shared_ptr<ContextBackend> backend)
{
	if(!backend || backend->id().isEmpty()) {
		return false;
	}
	const BackendId id = backend->id();
	{
		std::lock_guard<std::mutex> lock(m_registryMutex);
		if(m_backends.contains(id)) {
			qWarning(CAT_CONTROLLER) << "backend already registered:" << id;
			return false;
		}
		m_backends.insert(id, backend);
	}
	Q_EMIT backendRegistered(id);

	// Late registration: offer the view to connections already open.
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	for(ConnEntry &e : m_connections) {
		if(e.conn->kind() != backend->connectionKind() || e.conn->backends().contains(id)) {
			continue;
		}
		if(backend->maxThreads() > 0 && backend->maxThreads() < e.threads) {
			qWarning(CAT_CONTROLLER) << id << "needs" << backend->maxThreads() << "thread(s) but"
						 << e.conn->uri() << "runs" << e.threads;
			continue;
		}
		if(backend->compatible(*e.conn)) {
			e.conn->setBackends(e.conn->backends() << id);
			Q_EMIT backendsChanged(e.conn);
		}
	}
	return true;
}

bool Controller::unregisterBackend(const BackendId &id)
{
	std::lock_guard<std::mutex> ctxLock(m_ctxMutex);
	for(const ConnEntry &e : std::as_const(m_connections)) {
		if(e.viewRefs.contains(id)) {
			qWarning(CAT_CONTROLLER) << "backend" << id << "still has open views";
			return false;
		}
	}
	{
		std::lock_guard<std::mutex> lock(m_registryMutex);
		if(m_backends.remove(id) == 0) {
			return false;
		}
	}
	for(ConnEntry &e : m_connections) {
		QList<BackendId> list = e.conn->backends();
		if(list.removeAll(id) > 0) {
			e.conn->setBackends(list);
			Q_EMIT backendsChanged(e.conn);
		}
	}
	Q_EMIT backendUnregistered(id);
	return true;
}

QList<BackendId> Controller::backends() const
{
	std::lock_guard<std::mutex> lock(m_registryMutex);
	return m_backends.keys();
}

std::shared_ptr<ContextBackend> Controller::backend(const BackendId &id) const
{
	std::lock_guard<std::mutex> lock(m_registryMutex);
	return m_backends.value(id);
}

std::shared_ptr<ContextBackend> Controller::_resolveBackend(const BackendId &backend) const
{
	return this->backend(backend.isEmpty() ? BackendId(iio::IIOCtxBackend::ID) : backend);
}

QList<std::shared_ptr<ContextBackend>> Controller::_backendsOfKind(const ConnectionKind &kind) const
{
	std::lock_guard<std::mutex> lock(m_registryMutex);
	QList<std::shared_ptr<ContextBackend>> list;
	for(const auto &b : m_backends) {
		if(b->connectionKind() == kind) {
			list.append(b);
		}
	}
	return list;
}

QVector<ScanResult> Controller::scan(const ConnectionKind &kind, const QString &filter) const
{
	std::shared_ptr<ConnectionProvider> p = provider(kind);
	return p ? p->scan(filter) : QVector<ScanResult>();
}

DeviceMonitor *Controller::monitor(const ConnectionKind &kind) const
{
	std::shared_ptr<ConnectionProvider> p = provider(kind);
	return p ? p->monitor() : nullptr;
}

// ---- connections ----

Controller::ConnMap::iterator Controller::_openLocked(const QString &uri, const ConnectionKind &kind, int maxThreads)
{
	std::shared_ptr<ConnectionProvider> p = provider(kind);
	if(!p) {
		qWarning(CAT_CONTROLLER) << "no provider for" << kind;
		return m_connections.end();
	}
	if(auto it = m_connections.find(uri); it != m_connections.end()) {
		if(it->conn->kind() != p->kind()) {
			qWarning(CAT_CONTROLLER)
				<< uri << "is already open as" << it->conn->kind() << "not" << p->kind();
			return m_connections.end();
		}
		return it;
	}

	Connection *conn = p->open(uri);
	if(!conn) {
		return m_connections.end();
	}
	conn->setParent(this);

	QList<BackendId> compatible;
	int threads = std::max(1, maxThreads);
	for(const auto &b : _backendsOfKind(conn->kind())) {
		if(b->compatible(*conn)) {
			compatible.append(b->id());
			if(b->maxThreads() > 0) {
				threads = std::min(threads, b->maxThreads());
			}
		}
	}
	conn->setBackends(compatible);
	conn->setExecutor(new PooledCmdExecutor(threads, conn));
	conn->start();
	connect(conn, &Connection::lost, this, [this, conn]() { Q_EMIT connectionLost(conn); });

	auto it = m_connections.insert(uri, ConnEntry{conn, 0, threads, {}});
	Q_EMIT connectionOpened(conn);
	return it;
}

void Controller::_closeIfUnused(ConnMap::iterator it)
{
	if(it->connRefs > 0 || !it->viewRefs.isEmpty()) {
		return;
	}
	Connection *conn = it->conn;
	m_connections.erase(it);
	Q_EMIT connectionClosed(conn);
	conn->deleteLater();
}

ConnectionHandle Controller::_open(const QString &uri, const ConnectionKind &kind, int maxThreads)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto it = _openLocked(uri, kind, maxThreads);
	if(it == m_connections.end()) {
		return ConnectionHandle();
	}
	it->connRefs++;
	return ConnectionHandle(uri, it->conn);
}

void Controller::_releaseConnection(const QString &uri)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto it = m_connections.find(uri);
	if(it == m_connections.end()) {
		return;
	}
	it->connRefs--;
	_closeIfUnused(it);
}

// ---- views ----

ContextHandle Controller::_connectCtx(const QString &uri, const BackendId &backend, int maxThreads)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	std::shared_ptr<ContextBackend> b = _resolveBackend(backend);
	if(!b) {
		qWarning(CAT_CONTROLLER) << "unknown backend" << backend;
		return ContextHandle();
	}
	auto it = _openLocked(uri, b->connectionKind(), maxThreads);
	if(it == m_connections.end()) {
		return ContextHandle();
	}
	const BackendId id = b->id();
	Connection *conn = it->conn;
	if(!conn->backends().contains(id)) {
		qWarning(CAT_CONTROLLER) << id << "is not compatible with" << uri;
		_closeIfUnused(it);
		return ContextHandle();
	}
	Context *ctx = conn->context(id);
	if(!ctx) {
		ctx = b->create(*conn);
		if(!ctx) {
			qWarning(CAT_CONTROLLER) << id << "failed to build a view on" << uri;
			_closeIfUnused(it);
			return ContextHandle();
		}
		ctx->setUri(uri);
		conn->add(id, ctx);
		Q_EMIT contextCreated(conn, ctx);
	}
	it->viewRefs[id]++;
	return ContextHandle(uri, id, ctx);
}

ContextHandle Controller::_acquireExisting(const QString &uri, const BackendId &backend)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto it = m_connections.find(uri);
	if(it == m_connections.end()) {
		return ContextHandle();
	}
	const BackendId id = backend.isEmpty() ? BackendId(iio::IIOCtxBackend::ID) : backend;
	Context *ctx = it->conn->context(id);
	if(!ctx) {
		return ContextHandle();
	}
	it->viewRefs[id]++;
	return ContextHandle(uri, id, ctx);
}

void Controller::_releaseView(const QString &uri, const BackendId &backend)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto it = m_connections.find(uri);
	if(it == m_connections.end() || !it->viewRefs.contains(backend)) {
		return;
	}
	if(--it->viewRefs[backend] > 0) {
		return;
	}
	it->viewRefs.remove(backend);
	Connection *conn = it->conn;
	if(Context *ctx = conn->take(backend)) {
		Q_EMIT contextDestroyed(conn, ctx);
		if(std::shared_ptr<ContextBackend> b = this->backend(backend)) {
			b->destroy(ctx);
		}
		ctx->deleteLater();
	}
	_closeIfUnused(it);
}

ContextHandle Controller::adopt(const QString &uri, Context *ctx, const BackendId &backend)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	if(!ctx || backend.isEmpty() || m_connections.contains(uri)) {
		return ContextHandle();
	}
	auto *conn = new Connection(uri, backend, this);
	conn->setBackends({backend});
	ctx->setUri(uri);
	conn->add(backend, ctx);
	auto it = m_connections.insert(uri, ConnEntry{conn, 0, 1, {{backend, 1}}});
	Q_EMIT connectionOpened(conn);
	Q_EMIT contextCreated(conn, ctx);
	return ContextHandle(uri, backend, it->conn->context(backend));
}
