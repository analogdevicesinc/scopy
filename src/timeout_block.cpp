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

#include "timeout_block.hpp"

using namespace adiscope;

timeout_block::timeout_block(const std::string &pmt_name, QObject *parent)
	: QObject(parent)
	, gr::block("timeout_block", gr::io_signature::make(0, 0, 0), gr::io_signature::make(0, 0, 0)) {
	pmt::pmt_t port_id = pmt::mp(pmt_name);

	message_port_register_in(port_id);

	set_msg_handler(port_id, boost::bind(&timeout_block::pmt_received, this, _1));
}

timeout_block::~timeout_block() {}

void timeout_block::pmt_received(pmt::pmt_t pmt) {
	if (pmt::symbol_to_string(pmt).compare("timeout") == 0)
		Q_EMIT timeout();
}
