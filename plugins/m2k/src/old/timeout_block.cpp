/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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

#include "timeout_block.hpp"

using namespace scopy;

timeout_block::timeout_block(const std::string &pmt_name, QObject *parent) :
	QObject(parent), gr::block("timeout_block",
			gr::io_signature::make(0, 0, 0),
			gr::io_signature::make(0, 0, 0))
{
	pmt::pmt_t port_id = pmt::mp(pmt_name);

	message_port_register_in(port_id);

	set_msg_handler(port_id, std::bind(
				&timeout_block::pmt_received, this, std::placeholders::_1));
}

timeout_block::~timeout_block()
{
}

void timeout_block::pmt_received(pmt::pmt_t pmt)
{
	if (pmt::symbol_to_string(pmt).compare("timeout") == 0)
		Q_EMIT timeout();
}
