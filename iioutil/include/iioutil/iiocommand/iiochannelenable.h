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

#ifndef IIOCHANNELENABLE_H
#define IIOCHANNELENABLE_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioChannelEnable : public Command
{
	Q_OBJECT
public:
	explicit IioChannelEnable(struct iio_channel *channel, bool enable, QObject *parent)
		: m_channel(channel)
		, m_enable(enable)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		if(m_enable) {
			iio_channel_enable(m_channel);
		} else {
			iio_channel_disable(m_channel);
		}
		m_cmdResult->errorCode = 0;
		Q_EMIT finished(this);
	}

private:
	struct iio_channel *m_channel;
	const char *m_attribute_name;
	bool m_enable;
};
} // namespace scopy

#endif // IIOCHANNELENABLE_H
