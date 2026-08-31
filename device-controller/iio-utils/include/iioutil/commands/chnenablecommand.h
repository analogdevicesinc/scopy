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

#pragma once

#include "core/command.h"
#include "iioutil/ichannelops.h"

namespace scopy::iio {

class ChnEnableCommand : public Command
{
	Q_OBJECT
public:
	ChnEnableCommand(IChannelOps *ops, ChannelHandle handle, ChannelsMaskHandle mask, QObject *parent = nullptr)
		: Command(handle.ptr, parent)
		, m_ops(ops)
		, m_handle(handle)
		, m_mask(mask)
	{}

protected:
	void run() override { m_ops->enable(m_handle, m_mask); }

private:
	IChannelOps *m_ops;
	ChannelHandle m_handle;
	ChannelsMaskHandle m_mask;
};

} // namespace scopy::iio
