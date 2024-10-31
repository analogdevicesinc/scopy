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
