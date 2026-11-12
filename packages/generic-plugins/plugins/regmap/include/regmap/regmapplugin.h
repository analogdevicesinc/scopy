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
#include <QSet>

#include <pluginbase/registermaptoolconfig.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

#include <component/regmap/registerstrategies.h>

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

	// The generic register map as data: one entry per IIO device with register access that no plugin
	// configures, with regmap's own template and regmap-config.json options. Builds no widgets.
	QList<RegisterMapToolConfig> genericConfiguration(const QSet<QString> &covered = {});

	// Template and read/write are independent. An empty path means no template.
	// A null read/write means the default IIO strategy of that device. The strategies stay the caller's (from
	// the device controller); a write is read back when both come from the same place.
	bool configureTool(const QString &device, const QString &templatePath,
			   component::IRegisterReadStrategy *read = nullptr,
			   component::IRegisterWriteStrategy *write = nullptr);

	// Removes one device, or every device when called with no argument.
	void clearToolConfiguration(const QString &device = {});

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	// REGISTER_MAP_CONFIGURE_TOPIC for this device: pull the configuration again
	void messageCallback(QString topic, QString message) override;

private:
	// Asks the providers, adds the generic register map for the devices they don't configure, then rebuilds
	// every register map
	void reconfigure();
	bool isDongle() const;

	QWidget *m_registerMapWidget = nullptr;
	RegisterMapTool *registerMapTool = nullptr;
	void initApi();
	RegMap_API *m_api = nullptr;

	RegisterMapToolConfig genericEntry(const QString &devName, const QFileInfoList &xmlFiles);
	// One entry per device; between two providers the higher priority wins
	QList<RegisterMapToolConfig> resolveProviders(const QList<RegisterMapToolConfig> &provided);
	// Context device order as before, devices only a provider knows last
	QList<RegisterMapToolConfig> inContextOrder(QList<RegisterMapToolConfig> configs);
	bool isGeneric(const RegisterMapToolConfig &config) const;
	int providerPriority(const QString &provider);
	bool applyConfiguration(const RegisterMapToolConfig &config);
	// Takes the strategies of the access from the device controller. None gives two null strategies.
	bool findStrategies(const RegisterMapToolConfig &config, component::RegisterStrategies &strategies);
	bool findIioStrategies(const QString &device, uint32_t addressSpace, component::RegisterStrategies &strategies);
	// readBack: the reader and the writer are the same type, so a write is read back.
	// useRegmapConfig: the template's regmap-config.json options apply (generic and API paths, not plugin
	// entries). The swaps, when set, win over them.
	bool setupDevice(const QString &device, const QString &templatePath, int bitsPerRow,
			 component::IRegisterReadStrategy *read, component::IRegisterWriteStrategy *write,
			 bool readBack = true, bool useRegmapConfig = true,
			 std::optional<bool> useRegisterDescriptionAsName = {},
			 std::optional<bool> useBitFieldDescriptionAsName = {});
};
} // namespace regmap
} // namespace scopy

#endif // REGMAPPLUGIN_H
