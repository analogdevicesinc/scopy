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

#ifndef SAVERESTORETOOLSETTINGS_H
#define SAVERESTORETOOLSETTINGS_H

#include "m2ktool.hpp"

#include <QTemporaryFile>
#include <QtGlobal>

namespace scopy::m2k {
class SaveRestoreToolSettings
{
	SaveRestoreToolSettings(const SaveRestoreToolSettings &) = delete;
	SaveRestoreToolSettings &operator=(const SaveRestoreToolSettings &) = delete;

	SaveRestoreToolSettings(SaveRestoreToolSettings &&) = delete;
	SaveRestoreToolSettings &operator=(SaveRestoreToolSettings &&) = delete;

public:
	SaveRestoreToolSettings(M2kTool *tool);
	~SaveRestoreToolSettings();

private:
	QTemporaryFile m_temp;
	M2kTool *m_tool;
};
} // namespace scopy::m2k

#endif // SAVERESTORETOOLSETTINGS_H
