/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "pluginfilter.h"

#include <QJsonArray>

namespace scopy {
bool PluginFilter::pluginInCategory(Plugin *p, QString category)
{			       // PluginFilter class (?)
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

bool PluginFilter::pluginForcedInclusionList(QList<Plugin *> pl, Plugin *p)
{
	bool ret = false;
	QStringList includeList;
	for(Plugin *test : pl) {
		if(test->enabled() == false)
			continue;
		if(!test->metadata().contains("include-forced"))
			continue;
		QJsonValue includeVal = test->metadata().value("include-forced");
		if(includeVal.isString())
			includeList.append(includeVal.toString());
		if(includeVal.isArray()) {
			for(auto v : includeVal.toArray()) {
				if(!v.isString()) {
					continue;
				}
				includeList.append(v.toString());
			}
		}
	}

	for(const QString &include : includeList) {
		if(include.toLower() == p->name().toLower()) {
			ret = true;
		}
	}
	return ret;
}

bool PluginFilter::pluginInExclusionList(QList<Plugin *> pl, Plugin *p)
{
	bool ret = false;
	QStringList excludeList;
	for(Plugin *test : pl) {
		if(test->enabled() == false)
			continue;
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
			ret = true;
		}
		if(exclude.toLower() == p->name().toLower()) {
			ret = true;
		}
		if(exclude.toLower() == QString("!" + p->name()).toLower()) {
			ret = false;
			break;
		}
	}
	return ret;
}

} // namespace scopy
