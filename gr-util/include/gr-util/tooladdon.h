#ifndef TOOLADDON_H
#define TOOLADDON_H
#include "scopy-gr-util_export.h"
#include <QWidget>

namespace scopy {
class SCOPY_GR_UTIL_EXPORT ToolAddon {
public:
	virtual QString getName() = 0;
	virtual QWidget* getWidget() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void onStart() = 0;
	virtual void onStop() = 0;
	virtual void onInit() = 0;
	virtual void onDeinit() = 0;
	virtual void onChannelAdded(ToolAddon* ) = 0;
	virtual void onChannelRemoved(ToolAddon* ) = 0;
};

class SCOPY_GR_UTIL_EXPORT GRTopAddon {
public:
	virtual void preFlowStart() = 0;
	virtual void postFlowStart() = 0;
	virtual void preFlowStop() = 0;
	virtual void postFlowStop() = 0;
};
}

#define ToolAddon_iid "org.adi.Scopy.Plugins.ToolAddon/0.1"
#define GRTopAddon_iid "org.adi.Scopy.Plugins.GRTopAddon/0.1"
Q_DECLARE_INTERFACE(scopy::ToolAddon, ToolAddon_iid)
Q_DECLARE_INTERFACE(scopy::GRTopAddon, GRTopAddon_iid)

#endif // TOOLADDON_H
