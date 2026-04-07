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

#ifndef REGMAP_API_H
#define REGMAP_API_H

#include "scopy-regmap_export.h"

#include <regmapplugin.h>
#include <deviceregistermap.hpp>
#include <registermaptool.hpp>
#include <register/registermodel.hpp>
#include <registermapvalues.hpp>
#include <recyclerview/registermaptable.hpp>
#include <utils.hpp>
#include <register/bitfield/bitfieldmodel.hpp>
#include <readwrite/fileregisterreadstrategy.hpp>
#include <registermapsettingsmenu.hpp>
#include <registercontroller.hpp>
#include <register/registerdetailedwidget.hpp>
#include <qlineedit.h>
#include "search.hpp"

namespace scopy::regmap {

class SCOPY_REGMAP_EXPORT RegMap_API : public ApiObject
{

	Q_OBJECT

public:
	explicit RegMap_API(RegmapPlugin *regMapPlugin);
	~RegMap_API();

	Q_INVOKABLE void write(const QString &addr, const QString &val);
	Q_INVOKABLE void writeBitField(const QString &addr, const QString &val);
	Q_INVOKABLE QStringList getAvailableDevicesName();
	Q_INVOKABLE bool setDevice(const QString &device);
	Q_INVOKABLE QList<QString> search(const QString &searchParam);
	Q_INVOKABLE void readInterval(const QString &startAddr, const QString &stopAddr);
	Q_INVOKABLE bool enableAutoread(bool enable);
	Q_INVOKABLE bool isAutoreadEnabled();
	Q_INVOKABLE void registerDump(const QString &filePath);
	Q_INVOKABLE void setPath(const QString &filePath);
	Q_INVOKABLE void writeFromFile(const QString &filePath);
	Q_INVOKABLE QString readRegister(const QString &addr);
	Q_INVOKABLE QString getValueOfRegister(const QString &addr);
	Q_INVOKABLE QStringList getRegisterInfo(const QString &addr);
	Q_INVOKABLE QStringList getRegisterBitFieldsInfo(const QString &addr);
	Q_INVOKABLE QStringList getBitFieldInfo(const QString &addr, const QString &bitName);

private:
	RegmapPlugin *m_regMapPlugin;
	DeviceRegisterMap *getActiveDevRegMap();
};
} // namespace scopy::regmap
#endif // REGMAP_API_H
