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

#ifndef SERIALPORTENUMERATOR_H
#define SERIALPORTENUMERATOR_H

#include "scopy-iioutil_export.h"

#include <QMap>
#include <QString>

namespace scopy {
// Enumerates host serial ports via libserialport (no libiio). Used by the add-device
// serial section. Split out of the former IIOScanTask so serial scanning stays available
// after IIO scanning moved to the device-controller.
class SCOPY_IIOUTIL_EXPORT SerialPortEnumerator
{
public:
	static QMap<QString, QString> getSerialPortsName();
};
} // namespace scopy
#endif // SERIALPORTENUMERATOR_H
