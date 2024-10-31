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

#ifndef IIODEVICEATTRIBUTEWRITE_H
#define IIODEVICEATTRIBUTEWRITE_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioDeviceAttributeWrite : public Command
{
	Q_OBJECT
public:
	explicit IioDeviceAttributeWrite(struct iio_device *device, const char *attribute, const char *value,
					 QObject *parent, bool overwrite = false)
		: m_device(device)
		, m_attribute_name(std::string(attribute))
		, m_value(std::string(value))
	{
		setOverwrite(overwrite);
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		ssize_t ret = iio_device_attr_write(m_device, m_attribute_name.c_str(), m_value.c_str());
		m_cmdResult->errorCode = ret;
		Q_EMIT finished(this);
	}

	std::string getAttributeValue() { return m_value; }

private:
	struct iio_device *m_device;
	std::string m_attribute_name;
	std::string m_value;
};
} // namespace scopy

#endif // IIODEVICEATTRIBUTEWRITE_H
