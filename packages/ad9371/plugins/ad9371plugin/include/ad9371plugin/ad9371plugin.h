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
 */

#ifndef AD9371PLUGIN_H
#define AD9371PLUGIN_H

#define SCOPY_PLUGIN_NAME Ad9371Plugin

#include "scopy-ad9371plugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class Ad9371;
class Ad9371_API;

class SCOPY_AD9371PLUGIN_EXPORT Ad9371Plugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;
	friend class Ad9371_API;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	void initApi();
	IIOWidgetGroup *m_widgetGroup = nullptr;
	Ad9371 *m_ad9371Tool = nullptr;
	Ad9371_API *m_api = nullptr;
};
} // namespace scopy::ad9371
#endif // AD9371PLUGIN_H
