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

#ifndef DCDEBUGGERPLUGIN_H
#define DCDEBUGGERPLUGIN_H

#define SCOPY_PLUGIN_NAME DCDebuggerPlugin

#include "scopy-dcdebugger_export.h"

#include <QObject>
#include <QPointer>

#include <component/controller.h>
#include <pluginbase/pluginbase.h>

#include <vector>

namespace scopy::dcdebugger {
class DCDebuggerTool;

// Generic explorer over the device-controller tree of a connection: every compatible view
// (iio, capi, ...) is held while connected and browsed through the component bases only.
class SCOPY_DCDEBUGGER_EXPORT DCDebuggerPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	bool loadIcon() override;
	void loadToolList() override;
	QString description() override;
	QString pkgName() override;
	QString version() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	component::Connection *resolveConnection() const;
	void acquireViews(component::Connection *conn);
	void releaseViews();

	QPointer<component::Connection> m_conn;
	std::vector<component::ContextHandle> m_views;
	QPointer<DCDebuggerTool> m_tool;
};
} // namespace scopy::dcdebugger

#endif // DCDEBUGGERPLUGIN_H
