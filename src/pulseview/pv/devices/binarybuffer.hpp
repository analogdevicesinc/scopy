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


#ifndef PULSEVIEW_PV_DEVICE_BINARY_BUFFER_HPP
#define PULSEVIEW_PV_DEVICE_BINARY_BUFFER_HPP

#include <atomic>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "device.hpp"

namespace pv {
namespace devices {

class BinaryBuffer final : public Device
{
private:
    static const std::streamsize BufferSize;

public:    
    BinaryBuffer(const std::shared_ptr<sigrok::Context> &context,
                     short *data,
                     uint64_t number_of_samples,
        std::shared_ptr<sigrok::InputFormat> format,
        const std::map<std::string, Glib::VariantBase> &options);
    ~BinaryBuffer();

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

private:
    const std::shared_ptr<sigrok::Context> context_;
    const std::shared_ptr<sigrok::InputFormat> format_;
    const std::map<std::string, Glib::VariantBase> options_;
    short *data_;
    uint64_t no_samples_;
    std::shared_ptr<sigrok::Input> input_;

    std::ifstream *f;
    std::atomic<bool> interrupt_;
};

} // namespace devices
} // namespace pv

#endif // PULSEVIEW_PV_SESSIONS_BINARY_BUFFER_HPP

