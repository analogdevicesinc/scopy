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

#ifndef CRASHREPORT_H
#define CRASHREPORT_H

#include <QString>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT CrashReport
{
public:
	static void initSignalHandler();

private:
	static void signalHandler(int);
	static QString tmpFilePath_;
};
} // namespace scopy

#endif // CRASHREPORT_H
