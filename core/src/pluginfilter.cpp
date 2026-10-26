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
QStringList PluginFilter::pluginCategories(Plugin *p)
{
	QStringList list;
	QJsonValue val = p->metadata().value("category");
	if(val.isString()) {
		list.append(val.toString());
	} else if(val.isArray()) {
		for(auto v : val.toArray()) {
			if(v.isString()) {
				list.append(v.toString());
			}
		}
	}
	return list;
}

bool PluginFilter::pluginInCategories(Plugin *p, const QStringList &accepted)
{
	QStringList wanted = accepted;
	wanted.removeAll(QString());
	if(wanted.isEmpty()) {
		return true;
	}
	const QStringList categories = pluginCategories(p);
	if(categories.isEmpty()) {
		return true;
	}
	for(const QString &c : categories) {
		if(wanted.contains(c)) {
			return true;
		}
	}
	return false;
}

bool PluginFilter::pluginInCategory(Plugin *p, QString category) { return pluginInCategories(p, {category}); }

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
