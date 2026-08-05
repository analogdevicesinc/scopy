#pragma once

#include "component/context.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <memory>
#include <mutex>

namespace scopy::component {

class ContextFactory;

enum class BackendKind
{
	Libiiov0,
	Libiiov1,
	M2k,
	Audio,
	Default
};

class ContextHandle
{
public:
	ContextHandle() = default;
	ContextHandle(QString uri, Context *ctx)
		: m_uri(std::move(uri))
		, m_ctx(ctx)
	{
	}
	~ContextHandle() { reset(); }

	ContextHandle(const ContextHandle &) = delete;
	ContextHandle &operator=(const ContextHandle &) = delete;

	ContextHandle(ContextHandle &&o) noexcept
		: m_uri(std::move(o.m_uri))
		, m_ctx(std::exchange(o.m_ctx, nullptr))
	{
	}
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

class Controller : public QObject
{
	Q_OBJECT
protected:
	explicit Controller(QObject *parent = nullptr);
	~Controller() override;

public:
	Controller(const Controller &) = delete;
	void operator=(const Controller &) = delete;

	static Controller *GetInstance();

	// Register a factory for a backend kind. Called at construction for the
	// built-in kinds; extra kinds can be registered before connect().
	void registerFactory(BackendKind kind, std::shared_ptr<ContextFactory> factory);

    static ContextHandle connectCtx(const QString &uri, BackendKind backend)
    {
        Context *ctx = GetInstance()->_connectCtx(uri, backend);
        return ctx ? ContextHandle(uri, ctx) : ContextHandle();
    }

    static ContextHandle context(const QString &uri) { return GetInstance()->_acquireExisting(uri); }

	// Adopt an externally-built Context (used by tests with a fake tree).
    ContextHandle adopt(const QString &uri, Context *ctx);

Q_SIGNALS:
	void componentAdded(scopy::component::Context *ctx);
	void componentRemoved(scopy::component::Context *ctx);
	void connectionLost(scopy::component::Context *ctx);

private:
	friend class ContextHandle;

	struct CtxEntry
	{
		Context *ctx = nullptr;
		int refCount = 0;
	};

	Context *_connectCtx(const QString &uri, BackendKind backend);
	ContextHandle _acquireExisting(const QString &uri);
	void _disconnectCtx(const QString &uri);

	static Controller *pinstance_;
	static std::mutex mutex_;
	std::mutex m_ctxMutex;
    QMap<QString, CtxEntry> m_contexts;
	QMap<BackendKind, std::shared_ptr<ContextFactory>> m_factories;
};

} // namespace scopy::component
