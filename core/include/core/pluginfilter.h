#ifndef PLUGINFILTER_H
#define PLUGINFILTER_H
#include <pluginbase/plugin.h>


namespace adiscope {
class PluginFilter
{

private :
	PluginFilter();
public:
	static bool pluginInCategory(Plugin* p, QString category); // PluginFilter class (?)
	static bool pluginInExclusionList(QList <Plugin*> pl, Plugin *p);
};
}

#endif // PLUGINFILTER_H
