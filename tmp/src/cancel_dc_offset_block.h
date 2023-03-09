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
#ifndef CANCEL_DC_OFFSET_BLOCK_H
#define CANCEL_DC_OFFSET_BLOCK_H

#include <gnuradio/hier_block2.h>

#include "signal_sample.hpp"

namespace adiscope {
class cancel_dc_offset_block : public gr::hier_block2, public QObject
{
public:
        cancel_dc_offset_block(size_t buffer_size, bool enabled);
        ~cancel_dc_offset_block();

        void set_enabled(bool enabled);

	void set_buffer_size(size_t buffer_size);

	float get_dc_offset() const;

private:
        bool d_enabled;
        size_t d_buffer_size;
	float d_dc_offset;
	std::shared_ptr<signal_sample> d_signal;

        void _build_and_connect_blocks();

};
}

#endif // CANCEL_DC_OFFSET_BLOCK_H
