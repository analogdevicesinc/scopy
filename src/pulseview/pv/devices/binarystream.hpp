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

#ifndef PULSEVIEW_PV_DEVICE_BINARY_STREAM_HPP
#define PULSEVIEW_PV_DEVICE_BINARY_STREAM_HPP

#include <atomic>

#include <libsigrokcxx/libsigrokcxx.hpp>
#include "device.hpp"
#include <thread>
#include <mutex>
#include <memory>


extern "C" {
	struct iio_device;
	struct iio_buffer;
}
namespace adiscope {
	class LogicAnalyzer;
}

namespace pv {
namespace devices {

class BinaryStream final : public Device
{

public:
	BinaryStream(const std::shared_ptr<sigrok::Context> &context,
		     struct iio_device *data,
		     size_t buffersize,
		     std::shared_ptr<sigrok::InputFormat> format,
		     const std::map<std::string, Glib::VariantBase> &options,
		     adiscope::LogicAnalyzer* parent);
	~BinaryStream();

	void open();

	void close();

	/**
     * Builds the full name. It only contains all the fields.
     */
	std::string full_name() const;

	/**
     * Builds the display name. It only contains fields as required.
     */
	std::string display_name(const DeviceManager&) const;


	void start();

	void run();

	void stop();

	void set_buffersize(size_t value);

	size_t get_buffersize();

	void set_options(std::map<std::string, Glib::VariantBase> opt);

	std::map<std::string, Glib::VariantBase> get_options();

	void set_single(bool);

	void set_timeout(bool);
private:
	const std::shared_ptr<sigrok::Context> context_;
	const std::shared_ptr<sigrok::InputFormat> format_;
	std::map<std::string, Glib::VariantBase> options_;
	std::shared_ptr<sigrok::Input> input_;
	struct iio_buffer *data_;
	struct iio_device *dev_;
	void shutdown();
	std::ifstream *f;
	std::atomic<bool> interrupt_;
	size_t buffersize_;
	bool single_;
	std::atomic<bool> running;
	adiscope::LogicAnalyzer* la;
	bool autoTrigger;
	ssize_t nbytes_rx;
	mutable std::recursive_mutex data_mutex_;
};

} // namespace devices
} // namespace pv

#endif // PULSEVIEW_PV_SESSIONS_BinaryStream_HPP

