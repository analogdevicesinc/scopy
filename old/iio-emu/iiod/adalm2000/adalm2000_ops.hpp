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

#ifndef IIO_EMU_ADALM2000_OPS_HPP
#define IIO_EMU_ADALM2000_OPS_HPP

#include "iiod/abstract_ops.hpp"
#include "iiod/abstract_device.hpp"

namespace iio_emu {

class Adalm2000Ops : public AbstractOps{
public:
	Adalm2000Ops();
	~Adalm2000Ops() override;

	void setCurrentSocket(Socket *s) override;

private:
	static AbstractDevice *getDevice(const char *device_id);

	static ssize_t iio_get_xml(char **outxml);

	static ssize_t write(const char *buf, size_t len);

	static ssize_t read_line(char *buf, size_t len);

	static ssize_t read(char *buf, size_t len);

	static ssize_t iio_read_attr(const char *device_id, const char *attr, char *buf,
				     size_t len, enum iio_attr_type type);

	static ssize_t iio_write_attr(const char *device_id, const char *attr,
				      const char *buf,
				      size_t len, enum iio_attr_type type);

	static ssize_t iio_ch_read_attr(const char *device_id, const char *channel,
					bool ch_out, const char *attr, char *buf, size_t len);

	static ssize_t iio_ch_write_attr(const char *device_id, const char *channel,
					 bool ch_out, const char *attr, const char *buf, size_t len);

	static int32_t iio_open_dev(const char *device, size_t sample_size,
				    uint32_t mask, bool cyclic);

	static ssize_t iio_read_dev(const char *device, char *pbuf, size_t offset,
				    size_t bytes_count);

	static int32_t iio_set_buffers_count(const char *device, uint32_t buffers_count);

	static int32_t iio_get_mask(const char *device, uint32_t *mask);

	static ssize_t iio_transfer_mem_to_dev(const char *device, size_t bytes_count);

	static ssize_t iio_write_dev(const char *device, const char *buf,
				     size_t offset, size_t bytes_count);

	static int32_t iio_close_dev(const char *device);

};
}
#endif //IIO_EMU_ADALM2000_OPS_HPP
