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

#ifndef PQMPLUGIN_H
#define PQMPLUGIN_H

#define SCOPY_PLUGIN_NAME PQMPlugin

#include "scopy-pqm_export.h"

#include <QObject>
#include <acquisitionmanager.h>
#include <gui/infopage.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::pqm {
class SCOPY_PQM_EXPORT PQMPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
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
	void startPingTask() override;
	void stopPingTask() override;
	void onPausePingTask(bool pause) override;

private:
	void clearPingTask();

	InfoPage *m_infoPage;
	AcquisitionManager *m_acqManager;
};
} // namespace scopy::pqm

#endif // PQMPLUGIN_H
