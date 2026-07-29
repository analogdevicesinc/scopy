#pragma once

#include <QString>

namespace scopy::component {

class Context;

// Creates a fully built, executor-backed Context for one backend kind. Each
// concrete factory owns all backend-specific knowledge (loader, handle,
// builder), so the Controller stays backend-agnostic.
class ContextFactory
{
public:
	virtual ~ContextFactory() = default;
	virtual Context *create(const QString &uri) = 0;
};

} // namespace scopy::component
