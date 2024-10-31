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

#ifndef DEBUGGERPLUGIN_H
#define DEBUGGERPLUGIN_H

#define SCOPY_PLUGIN_NAME DebuggerPlugin

#include "QLabel"
#include "scopy-debugger_export.h"

#include <QObject>

#include "iioexplorerinstrument.h"

#include <pluginbase/pluginbase.h>
#include <iioutil/connection.h>

namespace scopy::debugger {
class SCOPY_DEBUGGER_EXPORT DebuggerPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	friend class IIODebugPlugin_API;
	void initPreferences() override;
	bool compatible(QString m_param, QString category) override;
	bool loadPreferencesPage() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;
	QString version() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	debugger::IIOExplorerInstrument *m_iioDebugger;
	Connection *m_conn;
	bool m_useDebuggerV2;
};
} // namespace scopy::debugger

#endif // DEBUGGERPLUGIN_H
