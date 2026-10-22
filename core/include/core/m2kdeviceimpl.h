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

#ifndef M2KDEVICEIMPL_H
#define M2KDEVICEIMPL_H

#include "deviceimpl.h"

namespace scopy {

/**
 * @brief ADALM2000 device: generic IIO context for plugin discovery, libm2k
 * context for everything after connect.
 */
class SCOPY_CORE_EXPORT M2kDeviceImpl : public DeviceImpl
{
	Q_OBJECT
public:
	explicit M2kDeviceImpl(QString param, int maxThreads = 1, QObject *parent = nullptr);
	~M2kDeviceImpl() override = default;

	bool verify() override;
	bool hasPendingBackendSwap() const { return m_swapPending; }

public Q_SLOTS:
	void connectDev() override;
	void disconnectDev() override;

private:
	void acquireGenericContext();

	int m_maxThreads = 4;
	bool m_swapPending = false;
};

} // namespace scopy

#endif // M2KDEVICEIMPL_H
