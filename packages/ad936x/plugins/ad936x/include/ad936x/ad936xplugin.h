/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef AD936XPLUGIN_H
#define AD936XPLUGIN_H

#define SCOPY_PLUGIN_NAME Ad936xPlugin

#include "scopy-ad936x_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::ad936x {
class SCOPY_AD936X_EXPORT Ad936xPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;
	QString displayName() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
};
} // namespace scopy::ad936x
#endif // AD936XPLUGIN_H
