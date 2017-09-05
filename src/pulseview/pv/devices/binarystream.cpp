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
#include "logic_analyzer.hpp"

using std::recursive_mutex;
using std::lock_guard;

namespace pv {
namespace devices {

BinaryStream::BinaryStream(const std::shared_ptr<sigrok::Context> &context,
			   struct iio_device *dev,
			   size_t buffersize,
			   std::shared_ptr<sigrok::InputFormat> format,
			   const std::map<std::string, Glib::VariantBase> &options,
			   adiscope::LogicAnalyzer* parent) :
	context_(context),
	dev_(dev),
	format_(format),
	options_(options),
	interrupt_(true),
	buffersize_(buffersize),
	single_(false),
	running(false),
	la(parent),
	autoTrigger(false),
        data_(nullptr),
        stream_mode(false),
        actual_buffersize(0)
{
	/* 10 buffers, 10ms each -> 250ms before we lose data */
	if(dev)
		iio_device_set_kernel_buffers_count(dev_, 25);
}

BinaryStream::~BinaryStream() {
	if (session_)
		close();
	input_.reset();

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
	if(dev_) {
		/*
		 * There is a restriction in the HDL that the buffer size must
		 * be a multiple of 8 bytes (4x 16-bit samples). Round up to the
		 * nearest multiple.
		 */
		actual_buffersize = ((buffersize_ + 3) / 4) * 4;
		data_ = iio_device_create_buffer(dev_, actual_buffersize, false);
	}

	if (!data_) {
		throw std::runtime_error("Could not create RX buffer");
	}
}

void BinaryStream::run()
{
	if(!dev_)
		return;
	if( running )
		stop();
	running = true;
	size_t nrx = 0;
	size_t size_to_display;
	input_->reset();
	interrupt_ = false;
        while (!interrupt_)
        {
                nbytes_rx = 0;
                size_to_display = 0;
                if(autoTrigger) {
                        la->startTimeout();
                }

                lock_guard<recursive_mutex> lock(data_mutex_);
                if(data_)
                        nbytes_rx = iio_buffer_refill(data_);

                if( nbytes_rx > 0 ) {
                        if( actual_buffersize != buffersize_ ) {
                                nbytes_rx -= ((actual_buffersize-buffersize_) * 2);
                        }

                        nrx += nbytes_rx / 2;
                        size_to_display = (nrx > entire_buffersize && !stream_mode) ?
                                                nbytes_rx-2*(nrx-entire_buffersize) : nbytes_rx;
                        if(data_)
                                input_->send(iio_buffer_start(data_), (size_t)(size_to_display));
                        la->bufferSentSignal(false);

                        if( nrx >= entire_buffersize && !stream_mode) {
                                size_t remaining_samples = 2 * (nrx - entire_buffersize);
                                if( !single_ ) {
                                        input_->end();
                                        if(data_ && remaining_samples > 0)
                                                input_->send(iio_buffer_start(data_)+(size_t)(size_to_display),
                                                     remaining_samples);
                                        nrx = 0;
                                        la->bufferSentSignal(true);
                                }
                        }

                        if(autoTrigger) {
                                la->stopTimeout();
                        }

                        if( single_ && running && nrx >= entire_buffersize) {
                                running = false;
                                interrupt_ = true;
                                stop();
                                nrx = 0;
                        }
                }
        }
        input_->end();
        interrupt_ = false;
        single_ = false;
}

void BinaryStream::set_timeout(bool checked)
{
	autoTrigger = checked;
}

void BinaryStream::set_buffersize(size_t value, bool entire_buf)
{
	buffersize_ = value;
	if(entire_buf)
		entire_buffersize =  value;
}

void BinaryStream::set_entire_buffersize(size_t value)
{
	entire_buffersize = value;
}

size_t BinaryStream::get_buffersize()
{
	return buffersize_;
}

void BinaryStream::set_single(bool check)
{
	if( running )
		stop();
	single_ = check;
}

bool BinaryStream::get_single()
{
	return single_;
}

bool BinaryStream::is_running()
{
	return running;
}

void BinaryStream::set_stream(bool check)
{
    stream_mode = check;
}

void BinaryStream::set_options(std::map<std::string, Glib::VariantBase> opt)
{
	options_ = opt;
}

std::map<std::string, Glib::VariantBase> BinaryStream::get_options()
{
	return options_;
}


/* cleanup and exit */
void BinaryStream::shutdown() {
	getchar();
	exit(0);
}

void BinaryStream::stop()
{
	interrupt_ = true;
	assert(session_);
	session_->stop();
	running = false;
	single_ = false;
	if(data_ )
		iio_buffer_cancel(data_);
	if( data_ )
	{
		iio_buffer_destroy(data_);
		data_ = nullptr;
	}
}

} // namespace devices
} // namespace pv
