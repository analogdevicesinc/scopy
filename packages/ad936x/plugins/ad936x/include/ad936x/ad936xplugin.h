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
#include <pluginbase/itoolconfigprovider.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <component/controller.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad936x {

class AD936X_API;
class AD936X_ADVANCED_API;
class FMCOMMS5_API;
class FMCOMMS5_ADVANCED_API;

class SCOPY_AD936X_EXPORT Ad936xPlugin : public QObject, public PluginBase, public IToolConfigProvider
{
	Q_OBJECT
	SCOPY_PLUGIN;
	Q_INTERFACES(scopy::IToolConfigProvider)

	friend class AD936X_API;
	friend class AD936X_ADVANCED_API;
	friend class FMCOMMS5_API;
	friend class FMCOMMS5_ADVANCED_API;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;
	QString displayName() override;

	// IToolConfigProvider: configures the register map tool of this device.
	// Uses only the parameter, the category and the IIO context, never this plugin's onConnect.
	QList<ToolConfiguration> toolConfiguration(const QString &toolId, const QVariantMap &query = {}) override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	void initApi();
	QList<ToolConfiguration> iioRegmapConfiguration();

	AD936X_API *m_api = nullptr;
	AD936X_ADVANCED_API *m_advancedApi = nullptr;
	FMCOMMS5_API *m_fmcomms5Api = nullptr;
	FMCOMMS5_ADVANCED_API *m_fmcomms5AdvancedApi = nullptr;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	bool m_isFmcomms5 = false;
	component::ContextHandle m_context;
};
} // namespace scopy::ad936x
#endif // AD936XPLUGIN_H
