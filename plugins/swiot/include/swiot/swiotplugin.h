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

#ifndef SWIOTPLUGIN_H
#define SWIOTPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTPlugin

#define AD74413R_TME_ID "swiotad74413r"
#define CONFIG_TME_ID "swiotconfig"
#define FAULTS_TME_ID "swiotfaults"
#define MAX14906_TME_ID "swiotmax14906"

#include "scopy-swiot_export.h"
#include "swiotcontroller.h"
#include "swiotruntime.h"
#include "swiotinfopage.h"
#include <QObject>
#include <QPushButton>
#include <tutorialbuilder.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::swiot {
class SCOPY_SWIOT_EXPORT SWIOTPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
	bool loadPage() override;
	bool loadExtraButtons() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString param, QString category) override;
	void initMetadata() override;
	QString description() override;
	QString pkgName() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	void startPingTask() override;
	void stopPingTask() override;
	void onPausePingTask(bool pause) override;

	void onIsRuntimeCtxChanged(bool isRuntimeCtx);

	void powerSupplyStatus(bool ps);

private Q_SLOTS:
	void setCtxMode(QString mode);
	void onModeAttributeChanged(QString mode);

private:
	void switchCtx();
	void setupToolList();
	void clearPingTask();
	void createStatusContainer();
	QPushButton *m_btnIdentify;
	QWidget *m_statusContainer;
	SwiotInfoPage *m_infoPage;

	QWidget *config;
	QWidget *adtool;
	QWidget *faults;
	QWidget *maxtool;

	SwiotController *m_swiotController;
	SwiotRuntime *m_runtime;

	bool m_isRuntime;
	bool m_switchCmd = false;
	QString m_ctxMode;

	const int PING_PERIOD = 2000;
};
} // namespace scopy::swiot
#endif // SWIOTPLUGIN_H
