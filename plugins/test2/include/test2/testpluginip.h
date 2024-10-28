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

#ifndef TESTPLUGINIP_H
#define TESTPLUGINIP_H

#define SCOPY_PLUGIN_NAME TestPluginIp

#include "pluginbase/pluginbase.h"
#include "scopy-test2_export.h"

#include <QObject>

#include <pluginbase/plugin.h>

namespace scopy {
class SCOPY_TEST2_EXPORT TestPluginIp : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN

	// Plugin interface
public:
	void postload() override;
	bool loadIcon() override;
	bool loadPage() override;
	void loadToolList() override;
	bool compatible(QString m_param, QString category) override;
	void unload() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void initMetadata() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

private:
	QWidget *m_tool;
};
} // namespace scopy
#endif // TESTPLUGINIP_H
