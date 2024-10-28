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

#ifndef SCOPYCONFIG_H
#define SCOPYCONFIG_H

#include "scopy-common_export.h"

#include <QString>

namespace scopy {

class SCOPY_COMMON_EXPORT config
{
public:
	static QString tempLogFilePath();
	static QString defaultPluginFolderPath();
	static QString localPluginFolderPath();
	static QString defaultTranslationFolderPath();
	static QString localTranslationFolderPath();
	static QString preferencesFolderPath();
	static QString settingsFolderPath();
	static QString executableFolderPath();
	static QString version();
	static QString gitCommit();
	static QString fullversion();
	static QString os();
	static QString pcSpecs();

	static QString dump();
	static QString getUuid();

private:
	inline static int uuid = 0;
};
} // namespace scopy
#endif // SCOPYCONFIG_H
