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

#ifndef DONGLEDEVICEMANAGER_H
#define DONGLEDEVICEMANAGER_H

#include "scopy-core_export.h"

#include <QObject>
#include <QString>

namespace scopy {
class DeviceManager;

/**
 * @brief Finds the dongleDevice and adds it on Home as a non-IIO device (category "dongle").
 * The plugins compatible with the "dongle" category decide what the device shows; the EEPROM configuration
 * plugin reads the EEPROM of the board behind the dongle and configures the other tools.
 * POC: the dongle is simulated by the dongle.json of the eprom_test package.
 * Later (if the POC is approved): find the boards through the real dongle (M2K now, M3K later).
 */
class SCOPY_CORE_EXPORT DongleDeviceManager : public QObject
{
	Q_OBJECT
public:
	static inline const QString CATEGORY = "dongle";

	explicit DongleDeviceManager(DeviceManager *dm, QObject *parent = nullptr);

	// Adds the simulated dongleDevice on Home, only when the eprom_test package is installed.
	void addSimulatedDevices();

private:
	DeviceManager *m_dm;
};
} // namespace scopy

#endif // DONGLEDEVICEMANAGER_H
