#pragma once

#include "core/icmdexecutor.h"

namespace scopy::component {

class Context;

// The one place that knows a backend's topology. The Controller creates the root
// Context + executor, then the builder fills in the subtree.
class ComponentBuilder
{
public:
	virtual ~ComponentBuilder() = default;
	virtual bool build(Context *ctx, ICmdExecutor *executor) = 0;
};

} // namespace scopy::component
