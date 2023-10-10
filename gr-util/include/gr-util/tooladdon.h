#ifndef TOOLADDON_H
#define TOOLADDON_H
#include "scopy-gr-util_export.h"

#include <QWidget>

namespace scopy {
class SCOPY_GR_UTIL_EXPORT ToolAddon
{
public:
	virtual QString getName() = 0;
	virtual QWidget *getWidget() = 0;

	virtual void enable() {}
	virtual void disable() {}
	virtual void onStart() {}
	virtual void onStop() {}
	virtual void onInit() {}
	virtual void onDeinit() {}
	virtual void onChannelAdded(ToolAddon *) {}
	virtual void onChannelRemoved(ToolAddon *) {}
};

class SCOPY_GR_UTIL_EXPORT GRTopAddon
{
public:
	virtual void preFlowBuild() {}
	virtual void postFlowBuild() {}
	virtual void preFlowStart() {}
	virtual void postFlowStart() {}
	virtual void preFlowStop() {}
	virtual void postFlowStop() {}
	virtual void preFlowTeardown() {}
	virtual void postFlowTeardown() {}
};
} // namespace scopy

#define ToolAddon_iid "org.adi.Scopy.Plugins.ToolAddon/0.1"
#define GRTopAddon_iid "org.adi.Scopy.Plugins.GRTopAddon/0.1"
Q_DECLARE_INTERFACE(scopy::ToolAddon, ToolAddon_iid)
Q_DECLARE_INTERFACE(scopy::GRTopAddon, GRTopAddon_iid)

#endif // TOOLADDON_H
