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

#include "component/channel.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// IIO channel identity. Carries the libiio channel handle on top of the shared
// Channel identity (id/name/label/direction).
class IIOChannel : public Channel
{
	Q_OBJECT
public:
	explicit IIOChannel(QObject *parent = nullptr)
		: Channel(parent)
	{}

	scopy::iio::ChannelHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::ChannelHandle handle) { m_handle = handle; }

private:
	scopy::iio::ChannelHandle m_handle;
};

} // namespace scopy::component::iio
