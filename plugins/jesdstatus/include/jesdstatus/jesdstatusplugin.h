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

#ifndef JESDSTATUSPLUGIN_H
#define JESDSTATUSPLUGIN_H

#define SCOPY_PLUGIN_NAME JesdStatusPlugin

#include "scopy-jesdstatus_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::jesdstatus {
class SCOPY_JESDSTATUS_EXPORT JesdStatusPlugin : public QObject, public PluginBase
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

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	struct iio_context *m_ctx;
	QList<QString> scanCompatibleDevices(iio_context *ctx);
};
} // namespace scopy::jesdstatus
#endif // JESDSTATUSPLUGIN_H
