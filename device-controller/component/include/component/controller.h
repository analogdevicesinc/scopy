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

    Context *_connectCtx(const QString &uri, BackendKind backend);
    static Context *connectCtx(const QString &uri, BackendKind backend)
    {
        return GetInstance()->_connectCtx(uri, backend);
    }

    void _disconnectCtx(const QString &uri);
    static void disconnectCtx(const QString &uri) { GetInstance()->_disconnectCtx(uri); }

    QMap<QString, Context *> contexts() const
    {
        QMap<QString, Context *> out;
        for(auto it = m_contexts.constBegin(); it != m_contexts.constEnd(); ++it) {
            out.insert(it.key(), it.value().ctx);
        }
        return out;
    }

	// Adopt an externally-built Context (used by tests with a fake tree).
    void adopt(const QString &uri, Context *ctx);

Q_SIGNALS:
	void componentAdded(scopy::component::Context *ctx);
	void componentRemoved(scopy::component::Context *ctx);
	void connectionLost(scopy::component::Context *ctx);

private:
	struct CtxEntry
	{
		Context *ctx = nullptr;
		int refCount = 0;
	};

	static Controller *pinstance_;
	static std::mutex mutex_;
	std::mutex m_ctxMutex;
    QMap<QString, CtxEntry> m_contexts;
	QMap<BackendKind, std::shared_ptr<ContextFactory>> m_factories;
};

} // namespace scopy::component
