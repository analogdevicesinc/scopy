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

#ifndef PLUGINREPOSITORY_H
#define PLUGINREPOSITORY_H

#include "pluginmanager.h"
#include "scopy-core_export.h"

#include <QMap>
#include <QObject>

namespace scopy {

class SCOPY_CORE_EXPORT PluginRepository : public QObject
{
	Q_OBJECT
protected:
	explicit PluginRepository(QObject *parent);
	~PluginRepository();

public:
	PluginRepository(PluginRepository &other) = delete;
	void operator=(const PluginRepository &) = delete;
	static PluginRepository *GetInstance();

	static void init(QString location = "");
	void _init(QString location);

	static PluginManager *getPluginManager();
	PluginManager *_getPluginManager();

	static QList<Plugin *> getOriginalPlugins();
	static QList<Plugin *> getPlugins(QString category = "");
	static QList<Plugin *> getCompatiblePlugins(const QString &param, const QString &category = "");
	static QList<PluginInfo> getPluginsInfo();

private:
	static PluginRepository *pinstance_;
	PluginManager *pm;
	QJsonObject metadata;
};
} // namespace scopy

#endif // PLUGINREPOSITORY_H
