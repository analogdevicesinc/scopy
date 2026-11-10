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

#ifndef EPROMTESTPLUGIN_H
#define EPROMTESTPLUGIN_H

#define SCOPY_PLUGIN_NAME EpromTestPlugin

#include "scopy-epromtest_export.h"

#include <QObject>

#include <pluginbase/registermaptoolconfig.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {

class EpromConfigTool;
class EpromTest_API;

/**
 * @brief Regmap POC: the EEPROM configuration plugin of the simulated dongleDevice.
 * Sync reads the EEPROM of the board behind the dongle and builds the configuration structure.
 * Configure applies it: regmap is asked to pull its configuration again and gets the synced entries.
 * Switch device simulates the board being replaced; Sync + Configure then reconfigure regmap while connected.
 */
class SCOPY_EPROMTEST_EXPORT EpromTestPlugin : public QObject, public PluginBase, public IRegisterMapConfigProvider
{
	Q_OBJECT
	SCOPY_PLUGIN;
	Q_INTERFACES(scopy::IRegisterMapConfigProvider)

	friend class EpromTest_API;

public:
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	void loadToolList() override;
	QString displayName() override;

	// IRegisterMapConfigProvider: the entries applied by the last Configure, empty before
	QList<RegisterMapToolConfig> registerMapConfiguration(const QVariantMap &query = {}) override;

	// Reads the board EEPROM and builds the configuration structure. False if the EEPROM can't be read.
	bool sync();
	// Applies the synced structure to the register map. False if nothing was synced.
	bool configure();
	// The synced structure as indented JSON, empty before sync()
	QString structure() const;
	// Simulates the board behind the dongle being replaced (board 1 <-> board 2). Sync again to read it.
	void switchDevice();
	// The board currently behind the dongle
	QString board() const;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	void initApi();

	int m_board = 0;
	QString m_syncedBoard; // the board name read from the EEPROM by the last Sync
	QList<RegisterMapToolConfig> m_synced;
	QList<RegisterMapToolConfig> m_applied;
	EpromConfigTool *m_tool = nullptr;
	EpromTest_API *m_api = nullptr;
};

} // namespace scopy

#endif // EPROMTESTPLUGIN_H
