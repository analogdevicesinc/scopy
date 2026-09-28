/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef AD9361REGMAPPLUGIN_H
#define AD9361REGMAPPLUGIN_H

#define SCOPY_PLUGIN_NAME Ad9361RegmapPlugin

#include "scopy-ad9361regmap_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {
namespace ad9361regmap {

// Tool-less plugin that configures the Register Map plugin for ad9361-phy (same pattern as rfpowermeter):
// register values are read from / written to a file and the register map XML comes from this package.
// Priority is lower than RegmapPlugin so onConnect runs after the register map tool is created.
class SCOPY_AD9361REGMAP_EXPORT Ad9361RegmapPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	QString findPackageFile(const QString &fileName) const;
	QString prepareValuesFile() const;
};
} // namespace ad9361regmap
} // namespace scopy
#endif // AD9361REGMAPPLUGIN_H
