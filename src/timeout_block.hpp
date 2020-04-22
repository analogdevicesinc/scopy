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

#ifndef TIMEOUT_BLOCK_HPP
#define TIMEOUT_BLOCK_HPP

#include <gnuradio/block.h>

#include <QObject>

namespace adiscope {
class timeout_block : public QObject, public gr::block
{
	Q_OBJECT

private:
	void pmt_received(pmt::pmt_t pmt);

public:
	explicit timeout_block(const std::string& pmt_name, QObject* parent = nullptr);
	~timeout_block();

Q_SIGNALS:
	void timeout();
};
} // namespace adiscope

#endif
