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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "scopy-core_export.h"

#include <QList>
#include <QObject>

#include <pluginbase/plugin.h>

namespace scopy {
class SCOPY_CORE_EXPORT PluginManager : public QObject
{
	Q_OBJECT
public:
	PluginManager(QObject *parent = nullptr);
	~PluginManager();
	void add(QStringList pluginFileList);
	void add(QString pluginFileName);
	int count();
	void sort();
	void clear();
	QList<Plugin *> getOriginalPlugins() const;
	QList<Plugin *> getPlugins(QString category = "");
	QList<Plugin *> getCompatiblePlugins(QString param, QString category = "");
	void setMetadata(QJsonObject metadata);
	QJsonObject metadata() const;

private:
	Plugin *loadPlugin(QString file);
	QList<Plugin *> list;
	QJsonObject m_metadata;

	void applyMetadata(Plugin *plugin, QJsonObject *metadata);
	bool pluginInCategory(Plugin *p, QString category);
};
} // namespace scopy
#endif // PLUGINMANAGER_H
