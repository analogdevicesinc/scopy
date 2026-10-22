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

class COMPONENT_EXPORT ContextHandle
{
public:
	ContextHandle() = default;
	ContextHandle(QString uri, Context *ctx)
		: m_uri(std::move(uri))
		, m_ctx(ctx)
	{}
	~ContextHandle() { reset(); }

	ContextHandle(const ContextHandle &) = delete;
	ContextHandle &operator=(const ContextHandle &) = delete;

	ContextHandle(ContextHandle &&o) noexcept
		: m_uri(std::move(o.m_uri))
		, m_ctx(std::exchange(o.m_ctx, nullptr))
	{}
	ContextHandle &operator=(ContextHandle &&o) noexcept
	{
		if(this != &o) {
			reset();
			m_uri = std::move(o.m_uri);
			m_ctx = std::exchange(o.m_ctx, nullptr);
		}
		return *this;
	}

	Context *get() const { return m_ctx; }
	Context *operator->() const { return m_ctx; }
	Context &operator*() const { return *m_ctx; }
	explicit operator bool() const { return m_ctx != nullptr; }

	void reset();

private:
	QString m_uri;
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

	// Backend registry. libiio is registered at construction; a package can register its own
	// backend at runtime (e.g. from Plugin::init()). Thread-safe.
	// Fails if the id is empty or already registered.
	bool registerBackend(std::shared_ptr<ContextBackend> backend);
	// Fails while contexts created by that backend are still open.
	bool unregisterBackend(const BackendId &id);
	QList<BackendId> backends() const;
	std::shared_ptr<ContextBackend> backend(const BackendId &id) const;

	// Blocking scan through one backend; call it off the UI thread.
	QVector<ScanResult> scan(const BackendId &id, const QString &filter = QString()) const;
	// The backend's auto-scan source, or nullptr.
	DeviceMonitor *monitor(const BackendId &id) const;

	// An empty backend means libiio. If uri is already open, its context is shared; requesting a
	// different backend for it fails.
	static ContextHandle connectCtx(const QString &uri, const BackendId &backend = {}, int maxThreads = 1)
	{
		Context *ctx = GetInstance()->_connectCtx(uri, backend, maxThreads);
		return ctx ? ContextHandle(uri, ctx) : ContextHandle();
	}

	static ContextHandle context(const QString &uri) { return GetInstance()->_acquireExisting(uri); }

	// Adopt an externally-built Context (used by tests with a fake tree).
	ContextHandle adopt(const QString &uri, Context *ctx);

Q_SIGNALS:
	void componentAdded(scopy::component::Context *ctx);
	void componentRemoved(scopy::component::Context *ctx);
	void connectionLost(scopy::component::Context *ctx);
	void backendRegistered(const QString &id);
	void backendUnregistered(const QString &id);

private:
	friend class ContextHandle;

	struct CtxEntry
	{
		Context *ctx = nullptr;
		int refCount = 0;
		BackendId backend;
	};

	Context *_connectCtx(const QString &uri, const BackendId &backend, int maxThreads);
	std::shared_ptr<ContextBackend> _resolveBackend(const BackendId &backend) const;
	ContextHandle _acquireExisting(const QString &uri);
	void _disconnectCtx(const QString &uri);

	static Controller *pinstance_;
	static std::mutex mutex_;
	// Lock order: m_ctxMutex, then m_backendsMutex.
	std::mutex m_ctxMutex;
	QMap<QString, CtxEntry> m_contexts;
	mutable std::mutex m_backendsMutex;
	QMap<BackendId, std::shared_ptr<ContextBackend>> m_backends;
};

} // namespace scopy::component
