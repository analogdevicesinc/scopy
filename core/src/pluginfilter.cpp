#include "pluginfilter.h"
#include <QJsonArray>

namespace scopy {
bool PluginFilter::pluginInCategory(Plugin* p, QString category) { // PluginFilter class (?)
	if(category.isEmpty()) // no category selected
		return true;
	if(!p->metadata().contains("category")) // plugin metadata does not have category
		return true;
	QJsonValue categoryVal = p->metadata().value("category");
	if(categoryVal.isString()) // single category
		return category == p->metadata().value("category").toString();
	if(categoryVal.isArray()) { // list category
		for(auto v : categoryVal.toArray()) {
			if(!v.isString()) {
				continue;
			}
			if(v.toString() == category) {
				return true;
			}

		}
	}
	return false;
}

bool PluginFilter::pluginInExclusionList(QList<Plugin*> pl, Plugin *p) {
	bool ret = true;
	QStringList excludeList;
	for(Plugin* test : pl) {
		if(!test->metadata().contains("exclude"))
			continue;
		QJsonValue excludeVal = test->metadata().value("exclude");
		if(excludeVal.isString())
			excludeList.append(excludeVal.toString());
		if(excludeVal.isArray()) {
			for(auto v : excludeVal.toArray()) {
				if(!v.isString()) {
					continue;
				}
				excludeList.append(v.toString());
			}
		}
	}

	for(const QString &exclude : excludeList) {
		if(exclude == "*") {
			ret = false;
		}
		if(exclude == p->name()) {
			ret = false;
		}
		if(exclude == "!"+p->name())
			ret = true;
	}
	return ret;
}

}
