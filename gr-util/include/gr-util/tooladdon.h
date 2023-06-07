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
	virtual void onAdd() = 0;
	virtual void onRemove() = 0;
	virtual void onChannelAdded(ToolAddon* ) = 0;
	virtual void onChannelRemoved(ToolAddon* ) = 0;
};
}
#endif // TOOLADDON_H
