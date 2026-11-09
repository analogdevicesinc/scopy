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

#ifndef EPROMTEST_API_H
#define EPROMTEST_API_H

#include "scopy-epromtest_export.h"

#include <QString>
#include <QStringList>

#include <pluginbase/apiobject.h>

namespace scopy {

class EpromTestPlugin;

// JS API "eprom_test": the Sync and Configure buttons of the EEPROM configuration tool
class SCOPY_EPROMTEST_EXPORT EpromTest_API : public ApiObject
{
	Q_OBJECT
public:
	explicit EpromTest_API(EpromTestPlugin *plugin);
	~EpromTest_API();

	Q_INVOKABLE QStringList getTools();
	// Same as pressing Sync. True if the board EEPROM was read.
	Q_INVOKABLE bool sync();
	// Same as pressing Configure. True if a synced structure was applied.
	Q_INVOKABLE bool configure();
	// The structure shown in the tool, empty before sync()
	Q_INVOKABLE QString getStructure();
	// Same as pressing Switch device: the board behind the dongle is replaced (board 1 <-> board 2)
	Q_INVOKABLE void switchDevice();
	// The board currently behind the dongle, e.g. "board 1"
	Q_INVOKABLE QString getBoard();

private:
	EpromTestPlugin *m_plugin;
};

} // namespace scopy

#endif // EPROMTEST_API_H
