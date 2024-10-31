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

#ifndef IIODEVICESETTRIGGER_H
#define IIODEVICESETTRIGGER_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceSetTrigger : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceSetTrigger(struct iio_device *device, struct iio_device *trigger_device, QObject *parent)
		: m_device(device)
		, m_trigger_device(trigger_device)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		ssize_t ret = iio_device_set_trigger(m_device, m_trigger_device);
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

private:
	struct iio_device *m_device;
	struct iio_device *m_trigger_device;
};
} // namespace scopy

#endif // IIODEVICESETTRIGGER_H
