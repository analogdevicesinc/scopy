#pragma once

#include "component/context.h"
#include "component/tags.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <memory>

namespace scopy::component {

class ContextFactory;

// Generic backend selector. Keeps the public API open for future backends
// (libm2k, audio) without adding a connect method per backend.
enum class BackendKind
{
	Libiiov0,
	Libiiov1,
	M2k,
	Audio,
	Default
};

// Top-level owner of all connected component trees. One Context root per
// connection; connect() selects the backend at runtime and builds the tree.
//
// TODO: the controller likely should own a single Context (one per connection).
// Revisit the multi-context list once integrated into Scopy's DeviceImpl.
class Controller : public QObject
{
	Q_OBJECT
public:
	explicit Controller(QObject *parent = nullptr);
	~Controller() override;

	// Register a factory for a backend kind. Called at construction for the
	// built-in kinds; extra kinds can be registered before connect().
	void registerFactory(BackendKind kind, std::shared_ptr<ContextFactory> factory);

	Context *connect(const QString &uri, BackendKind backend);
	void disconnect(Context *ctx);

	QList<Context *> contexts() const { return m_contexts; }

	template <typename T>
	QList<T *> findAll() const
	{
		QList<T *> out;
		for(Context *ctx : m_contexts) {
			out += ctx->findChildren<T *>();
		}
		return out;
	}

	template <typename T>
	QList<T *> findAllByTag(const QString &tag) const
	{
		QList<T *> out;
		for(Context *ctx : m_contexts) {
			out += findByTag<T>(ctx, tag, true);
		}
		return out;
	}

	// Adopt an externally-built Context (used by tests with a fake tree).
	void adopt(Context *ctx);

Q_SIGNALS:
	void componentAdded(scopy::component::Context *ctx);
	void componentRemoved(scopy::component::Context *ctx);
	void connectionLost(scopy::component::Context *ctx);

private:
	QList<Context *> m_contexts;
	QMap<BackendKind, std::shared_ptr<ContextFactory>> m_factories;
};

} // namespace scopy::component
