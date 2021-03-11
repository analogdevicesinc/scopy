/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IIO_EMU_ABSTRACT_DEVICE_IN_HPP
#define IIO_EMU_ABSTRACT_DEVICE_IN_HPP

#include <tinyiiod/compat.h>
#include "iiod/abstract_device.hpp"
#include "iiod/abstract_device_out.hpp"

namespace iio_emu {

class AbstractDeviceIn : public AbstractDevice{
public:
	virtual ssize_t read_dev(char *pbuf, size_t offset, size_t bytes_count) = 0;
	virtual void connectDevice(unsigned short channel_in, AbstractDeviceOut *deviceOut, unsigned short channel_out) = 0;
};
}

#endif //IIO_EMU_ABSTRACT_DEVICE_IN_HPP
