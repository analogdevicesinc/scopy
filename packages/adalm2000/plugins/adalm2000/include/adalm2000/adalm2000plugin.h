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
 */

#ifndef ADALM2000PLUGIN_H
#define ADALM2000PLUGIN_H

#define SCOPY_PLUGIN_NAME Adalm2000Plugin

#include "scopy-adalm2000_export.h"

#include <QObject>

#include <qcoro/qcorotask.h>

#include <component/controller.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::adalm2000 {

class SCOPY_ADALM2000_EXPORT Adalm2000Plugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void init() override;
	bool compatible(QString param, QString category) override;
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
	QCoro::Task<void> calibrateAsync();

	component::ContextHandle m_context;
	bool m_calibrated = false;
};

} // namespace scopy::adalm2000
#endif // ADALM2000PLUGIN_H
