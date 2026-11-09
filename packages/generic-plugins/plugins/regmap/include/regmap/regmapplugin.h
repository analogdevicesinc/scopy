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

#ifndef REGMAPPLUGIN_H
#define REGMAPPLUGIN_H

#define SCOPY_PLUGIN_NAME RegmapPlugin
#define SCOPY_PLUGIN_PRIO 100

#include "pluginbase/pluginbase.h"
#include "scopy-regmap_export.h"

#include <QFileInfo>
#include <QObject>

#include <pluginbase/itoolconfigprovider.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

#include "readwrite/iregisterreadstrategy.hpp"
#include "readwrite/iregisterwritestrategy.hpp"

namespace Ui {}

namespace scopy {
namespace regmap {

class RegisterMapTool;
class JsonFormatedElement;
class RegMap_API;

class SCOPY_REGMAP_EXPORT RegmapPlugin : public QObject, public PluginBase
{
	friend class RegMap_API;
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString uri, QString category) override;
	void initPreferences() override;
	bool loadPreferencesPage() override;

	void initMetadata() override;
	QString description() override;
	QString displayName() override;
	QString pkgName() override;

	QWidget *getTool();

	void generateDevice(QString xmlPath, QString devName, IRegisterReadStrategy *readStrategy,
			    IRegisterWriteStrategy *writeStrategy, int bitsPerRow = 8);

	// Today's default setup as data: one entry per IIO device with register access. Builds no widgets.
	QList<ToolConfiguration> defaultConfiguration();

	// Template and read/write are independent. An empty path means no template.
	// A null read/write means the default IIO strategy of that device. Regmap takes ownership.
	bool configureTool(const QString &device, const QString &templatePath, IRegisterReadStrategy *read = nullptr,
			   IRegisterWriteStrategy *write = nullptr);

	// Removes one device, or every device when called with no argument.
	void clearToolConfiguration(const QString &device = {});

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	// configkeys::CONFIGURE_TOPIC for this device: pull the configuration again
	void messageCallback(QString topic, QString message) override;

private:
	// Asks the providers, merges with the defaults, then rebuilds every register map
	void reconfigure();
	bool isDongle() const;

	QWidget *m_registerMapWidget = nullptr;
	RegisterMapTool *registerMapTool = nullptr;
	void initApi();
	RegMap_API *m_api = nullptr;

	ToolConfiguration defaultEntry(const QString &devName, const QFileInfoList &xmlFiles);
	QList<ToolConfiguration> mergeConfigurations(const QList<ToolConfiguration> &defaults,
						     const QList<ToolConfiguration> &provided);
	int providerPriority(const QString &provider);
	bool applyConfiguration(const ToolConfiguration &config);
	// Maps the "access" property to strategies. "none" gives two null strategies.
	bool createStrategies(const ToolConfiguration &config, IRegisterReadStrategy *&read,
			      IRegisterWriteStrategy *&write);
	bool createIioStrategies(const QString &device, uint32_t addressSpace, IRegisterReadStrategy *&read,
				 IRegisterWriteStrategy *&write);
	bool setupDevice(const QString &device, const QString &templatePath, int bitsPerRow,
			 IRegisterReadStrategy *read, IRegisterWriteStrategy *write);
};
} // namespace regmap
} // namespace scopy

#endif // REGMAPPLUGIN_H
