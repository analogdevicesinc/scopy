#ifndef PLUGINFILTER_H
#define PLUGINFILTER_H
#include <pluginbase/plugin.h>

namespace scopy {
class PluginFilter
{

private:
	PluginFilter();

public:
	static bool pluginInCategory(Plugin *p, QString category); // PluginFilter class (?)
	static bool pluginInExclusionList(QList<Plugin *> pl, Plugin *p);
	static bool pluginForcedInclusionList(QList<Plugin *> pl, Plugin *p);
};
} // namespace scopy

#endif // PLUGINFILTER_H
