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

#include <cassert>
#include <fstream>

#include <QString>
#include <QDebug>
#include "binarystream.hpp"
#include <iio.h>
#include <iostream>

namespace pv {
namespace devices {

BinaryStream::BinaryStream(const std::shared_ptr<sigrok::Context> &context,
			   struct iio_device *dev,
			   size_t buffersize,
			   std::shared_ptr<sigrok::InputFormat> format,
			   const std::map<std::string, Glib::VariantBase> &options) :
	context_(context),
	dev_(dev),
	format_(format),
	options_(options),
	interrupt_(false),
	buffersize_(buffersize),
	single_(false)
{
	/* 10 buffers, 10ms each -> 250ms before we lose data */
	iio_device_set_kernel_buffers_count(dev_, 25);
}

BinaryStream::~BinaryStream() {
	qDebug() << "binary stream destroyed\n";
}

void BinaryStream::open()
{
	if (session_)
		close();
	else
		session_ = context_->create_session();

	input_ = format_->create_input(options_);

	if (!input_)
		throw QString("Failed to create input");

	input_->send(data_, 0);

	try {
		device_ = input_->device();
	} catch (sigrok::Error) {
		return;
	}

	session_->add_device(device_);
}

void BinaryStream::close()
{
	if (session_)
		session_->remove_devices();
}


std::string BinaryStream::full_name() const
{
	return "logicanalyzer::full_name";
}

std::string BinaryStream::display_name(const DeviceManager&) const
{
	return "logicanalyzer::display_full_name";
}

void BinaryStream::start()
{
	/* sample_rate / 100 -> 10ms */
	data_ = iio_device_create_buffer(dev_, buffersize_, false);

	if (!data_) {
		throw std::runtime_error("Could not create RX buffer");
	}
}

void BinaryStream::run()
{
	size_t nrx = 0;
	input_->reset();
	interrupt_ = false;
	ssize_t nbytes_rx;
	while (!interrupt_)
	{
		nbytes_rx = iio_buffer_refill(data_);
		if (nbytes_rx < 0)
		{
			printf("Error refilling buf %d\n", (int)nbytes_rx);
			shutdown();
		}
		nrx += nbytes_rx / 2;
		input_->send(iio_buffer_start(data_), (size_t)(nbytes_rx));
		if( single_ )
			interrupt_ = true;
	}
	input_->end();
	interrupt_ = false;
	single_ = false;
}

void BinaryStream::set_buffersize(size_t value)
{
	close();
	buffersize_ = value;
	open();
}

void BinaryStream::set_single(bool check)
{
	single_ = check;
}

void BinaryStream::set_options(std::map<std::string, Glib::VariantBase> opt)
{
	close();
	options_ = opt;
	open();
}

/* cleanup and exit */
void BinaryStream::shutdown() {
	printf("SHUTDOWN");
	getchar();
	exit(0);
}

void BinaryStream::stop()
{
	interrupt_ = true;
	iio_buffer_destroy(data_);
	qDebug() << "binary stream stopped\n";
}

} // namespace devices
} // namespace pv
