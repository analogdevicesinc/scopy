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

#include "iioutil/handles.h"

#include <component/ping.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CONTROLLER, "Controller")

using namespace scopy::component;

void ContextHandle::reset()
{
	if(m_ctx) {
		Controller::GetInstance()->_disconnectCtx(m_uri);
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
	registerBackend(std::make_shared<iio::IIOCtxBackend>());
}

Controller::~Controller() = default;

bool Controller::registerBackend(std::shared_ptr<ContextBackend> backend)
{
	if(!backend || backend->id().isEmpty()) {
		return false;
	}
	const BackendId id = backend->id();
	{
		std::lock_guard<std::mutex> lock(m_backendsMutex);
		if(m_backends.contains(id)) {
			qWarning(CAT_CONTROLLER) << "backend already registered:" << id;
			return false;
		}
		m_backends.insert(id, std::move(backend));
	}
	Q_EMIT backendRegistered(id);
	return true;
}

bool Controller::unregisterBackend(const BackendId &id)
{
	{
		std::lock_guard<std::mutex> ctxLock(m_ctxMutex);
		for(const CtxEntry &e : std::as_const(m_contexts)) {
			if(e.backend == id) {
				qWarning(CAT_CONTROLLER) << "backend" << id << "still has open contexts";
				return false;
			}
		}
		std::lock_guard<std::mutex> lock(m_backendsMutex);
		if(m_backends.remove(id) == 0) {
			return false;
		}
	}
	Q_EMIT backendUnregistered(id);
	return true;
}

QList<BackendId> Controller::backends() const
{
	std::lock_guard<std::mutex> lock(m_backendsMutex);
	return m_backends.keys();
}

std::shared_ptr<ContextBackend> Controller::backend(const BackendId &id) const
{
	std::lock_guard<std::mutex> lock(m_backendsMutex);
	return m_backends.value(id);
}

QVector<ScanResult> Controller::scan(const BackendId &id, const QString &filter) const
{
	std::shared_ptr<ContextBackend> b = backend(id);
	return b ? b->scan(filter) : QVector<ScanResult>();
}

DeviceMonitor *Controller::monitor(const BackendId &id) const
{
	std::shared_ptr<ContextBackend> b = backend(id);
	return b ? b->monitor() : nullptr;
}

std::shared_ptr<ContextBackend> Controller::_resolveBackend(const BackendId &backend) const
{
	std::lock_guard<std::mutex> lock(m_backendsMutex);
	return m_backends.value(backend.isEmpty() ? BackendId(iio::IIOCtxBackend::ID) : backend);
}

Context *Controller::_connectCtx(const QString &uri, const BackendId &backend, int maxThreads)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	if(auto e = m_contexts.find(uri); e != m_contexts.end()) {
		if(!backend.isEmpty() && backend != e->backend) {
			qWarning(CAT_CONTROLLER)
				<< uri << "is already open with backend" << e->backend << "not" << backend;
			return nullptr;
		}
		e->refCount++;
		return e->ctx;
	}

	std::shared_ptr<ContextBackend> b = _resolveBackend(backend);
	if(!b) {
		return nullptr;
	}
	Context *ctx = b->create(uri, maxThreads);
	if(ctx) {
		ctx->setParent(this);
		m_contexts.insert(uri, CtxEntry{ctx, 1, b->id()});
		if(Ping *ping = ctx->findChild<Ping *>()) {
			connect(ping, &Ping::connectionLost, this, [this, ctx]() { Q_EMIT connectionLost(ctx); });
			ping->startMonitoring(2000);
		}
		Q_EMIT componentAdded(ctx);
	}
	return ctx;
}

ContextHandle Controller::adopt(const QString &uri, Context *ctx)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	if(!ctx || m_contexts.contains(uri)) {
		return ContextHandle();
	}
	ctx->setParent(this);
	m_contexts.insert(uri, CtxEntry{ctx, 1});
	Q_EMIT componentAdded(ctx);
	return ContextHandle(uri, ctx);
}

ContextHandle Controller::_acquireExisting(const QString &uri)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto e = m_contexts.find(uri);
	if(e == m_contexts.end()) {
		return ContextHandle();
	}
	e->refCount++;
	return ContextHandle(uri, e->ctx);
}

void Controller::_disconnectCtx(const QString &uri)
{
	std::lock_guard<std::mutex> lock(m_ctxMutex);
	auto e = m_contexts.find(uri);
	if(e == m_contexts.end()) {
		return;
	}
	if(--e->refCount > 0) {
		return;
	}
	Context *ctx = e->ctx;
	m_contexts.erase(e);
	Q_EMIT componentRemoved(ctx);
	ctx->deleteLater();
}
