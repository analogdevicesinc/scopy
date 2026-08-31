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

#include "component/backends/iio/iiocontextfactory.h"
#include "component/contextfactory.h"

#include "iioutil/handles.h"

#include <component/ping.h>

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
	registerFactory(BackendKind::Libiiov0, std::make_shared<iio::IIOContextFactory>(scopy::iio::LibiioVersion::V0));
	registerFactory(BackendKind::Libiiov1, std::make_shared<iio::IIOContextFactory>(scopy::iio::LibiioVersion::V1));
	registerFactory(BackendKind::Default,
			std::make_shared<iio::IIOContextFactory>(scopy::iio::LibiioVersion::Default));
}

Controller::~Controller() = default;

void Controller::registerFactory(BackendKind kind, std::shared_ptr<ContextFactory> factory)
{
	m_factories.insert(kind, std::move(factory));
}

Context *Controller::_connectCtx(const QString &uri, BackendKind backend)
{
	auto it = m_factories.constFind(backend);
	if(it == m_factories.constEnd() || !it.value()) {
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(m_ctxMutex);
	if(auto e = m_contexts.find(uri); e != m_contexts.end()) {
		e->refCount++;
		return e->ctx;
	}
	Context *ctx = it.value()->create(uri);
	if(ctx) {
		ctx->setParent(this);
		m_contexts.insert(uri, CtxEntry{ctx, 1});
		Ping *ping = ctx->findChild<Ping *>();
		connect(ping, &Ping::connectionLost, this, [this, ctx]() { Q_EMIT connectionLost(ctx); });
		ping->startMonitoring(2000);
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
