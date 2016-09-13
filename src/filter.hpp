/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>

#include <QJsonObject>

extern "C" {
	struct iio_context;
	struct iio_device;
}

namespace adiscope {

	enum tool {
		TOOL_OSCILLOSCOPE = 0,
		TOOL_SIGNAL_GENERATOR = 1,
		TOOL_DMM = 2,
		TOOL_POWER_CONTROLLER = 3,
	};

	class Filter
	{
	private:
		QJsonObject root;
		bool no_filter;

	public:
		Filter(const std::string &id);
		~Filter();

		bool compatible(enum tool tool) const;
		bool usable(enum tool tool, const std::string &dev) const;
		const std::string device_name(enum tool tool) const;

		struct iio_device * find_device(const struct iio_context *ctx,
				enum tool tool) const;
	};
}

#endif /* FILTER_HPP */
