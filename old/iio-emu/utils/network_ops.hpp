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

#ifndef IIO_EMU_NETWORK_OPS_HPP
#define IIO_EMU_NETWORK_OPS_HPP

#include "networking/socket.hpp"

namespace iio_emu {

ssize_t socket_read_line(Socket *socket, char *buf, size_t len);

ssize_t socket_read(Socket *socket, void *buf, size_t len);
}
#endif //IIO_EMU_NETWORK_OPS_HPP
