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

#ifndef REGMAPCONFIGKEYS_H
#define REGMAPCONFIGKEYS_H

#include <QStringList>

// Keys of a regmap ToolConfiguration (see pluginbase/itoolconfigprovider.h).
namespace scopy::regmap::configkeys {
inline const QString TOOL_ID = "regmap";

inline const QString TEMPLATE = "template";	     // register XML path, empty for no template
inline const QString ACCESS = "access";		     // one of the ACCESS_* values below
inline const QString FILE_PATH = "file";		     // CSV path when access is "file"
inline const QString BITS_PER_ROW = "bitsPerRow";    // optional, default from regmap-config.json
inline const QString ADDRESS_SPACE = "addressSpace"; // optional, default from regmap-config.json

inline const QString ACCESS_DEFAULT = "default"; // IIO read/write, as today
inline const QString ACCESS_FILE = "file";	 // CSV read/write
inline const QString ACCESS_NONE = "none";	 // template only

inline const QStringList ALL = {TEMPLATE, ACCESS, FILE_PATH, BITS_PER_ROW, ADDRESS_SPACE};
} // namespace scopy::regmap::configkeys

#endif // REGMAPCONFIGKEYS_H
