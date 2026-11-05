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

#ifndef MULTIACQPLUGIN_H
#define MULTIACQPLUGIN_H

#define SCOPY_PLUGIN_NAME MultiAcqPlugin

#include "scopy-multiacq_export.h"

#include <QObject>
#include <QPointer>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::multiacq {

class MultiAcqTool;

// A plugin that belongs to no device: compatible() is always false, so it is never cloned
// for one. Its one tool comes through globalToolList() instead, and connects to the devices
// it acquires from by itself, from URIs the reader types in.
class SCOPY_MULTIACQ_EXPORT MultiAcqPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString param, QString category) override;
	QList<ToolMenuEntry *> globalToolList() override;
	void initMetadata() override;
	QString description() override;
	QString pkgName() override;
	QString about() override;
	QString version() override;

public Q_SLOTS:
	// Never cloned for a device, so never connected.
	bool onConnect() override { return false; }
	bool onDisconnect() override { return true; }

private:
	QPointer<MultiAcqTool> m_tool;
};

} // namespace scopy::multiacq

#endif // MULTIACQPLUGIN_H
