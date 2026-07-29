#include "component/controller.h"

#include "component/backends/iio/iiocontextfactory.h"
#include "component/contextfactory.h"

#include "iioutil/handles.h"

using namespace scopy::component;

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

Context *Controller::connect(const QString &uri, BackendKind backend)
{
	auto it = m_factories.constFind(backend);
	if(it == m_factories.constEnd() || !it.value()) {
		return nullptr;
	}

	Context *ctx = it.value()->create(uri);
	if(ctx) {
		adopt(ctx);
	}
	return ctx;
}

void Controller::adopt(Context *ctx)
{
	if(!ctx || m_contexts.contains(ctx)) {
		return;
	}
	ctx->setParent(this);
	m_contexts.append(ctx);
	Q_EMIT componentAdded(ctx);
}

void Controller::disconnect(Context *ctx)
{
	if(!ctx || !m_contexts.removeOne(ctx)) {
		return;
	}
	Q_EMIT componentRemoved(ctx);
	ctx->deleteLater();
}
