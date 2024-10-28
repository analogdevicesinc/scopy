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

#ifndef IIOBUFFERDESTROY_H
#define IIOBUFFERDESTROY_H

#include "../command.h"

#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IioBufferDestroy : public Command
{
	Q_OBJECT
public:
	explicit IioBufferDestroy(struct iio_buffer *buffer, QObject *parent)
		: m_buffer(buffer)
	{
		this->setParent(parent);
		m_cmdResult = new CommandResult();
	}

	virtual void execute() override
	{
		Q_EMIT started(this);
		m_cmdResult->errorCode = 0;
		iio_buffer_destroy(m_buffer);
		if(!m_buffer) {
			m_cmdResult->errorCode = -errno;
		}
		Q_EMIT finished(this);
	}

private:
	struct iio_buffer *m_buffer;
};
} // namespace scopy

#endif // IIOBUFFERDESTROY_H
