/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <cassert>
#include <fstream>

#include <QString>

#include "binarybuffer.hpp"

namespace pv {
namespace devices {

const std::streamsize BinaryBuffer::BufferSize = 16384;

BinaryBuffer::BinaryBuffer(const std::shared_ptr<sigrok::Context> &context,
    short *data,
    uint64_t *number_of_samples,
    std::shared_ptr<sigrok::InputFormat> format,
    const std::map<std::string, Glib::VariantBase> &options) :

    context_(context),
    data_(data),
    no_samples_(number_of_samples),
    format_(format),
    options_(options)

{
    qDebug("PatternGeneratorDevice created");
}

BinaryBuffer::~BinaryBuffer()
{
    qDebug("PatternGeneratorDevice destroyed");
}

void BinaryBuffer::open()
{
    if (session_)
        close();
    else
        session_ = context_->create_session();

    input_ = format_->create_input(options_);

    if (!input_)
        throw QString("Failed to create input");

    // open() should add the input device to the session but
    // we can't open the device without sending some data first

    if ((*no_samples_) == 0)
        return;

    input_->send(data_, 1);

    try {
        device_ = input_->device();
    } catch (sigrok::Error) {
        return;
    }

    session_->add_device(device_);
}

void BinaryBuffer::close()
{
    if (session_)
        session_->remove_devices();
}


std::string BinaryBuffer::full_name() const
{
    return "patterngenerator::full_name";
}

std::string BinaryBuffer::display_name(const DeviceManager&) const
{
    return "patterngenerator::display_full_name";
}


void BinaryBuffer::start()
{
    qDebug("PatternGeneratorDevice started");
}

void BinaryBuffer::run()
{
    //char buffer[BufferSize];

    input_->reset();

    /*TODO: break down in multiple  data chunks */
    input_->send(data_, (*no_samples_));
    input_->end();


}

void BinaryBuffer::stop()
{
    qDebug("PatternGeneratorDevice stopped");
}

} // namespace devices
} // namespace pv
