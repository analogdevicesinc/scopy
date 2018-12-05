#include "cancel_dc_offset_block.h"

#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>
#include <gnuradio/blocks/moving_average_ff.h>
#include <gnuradio/blocks/sub_ff.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/repeat.h>
#include <gnuradio/blocks/copy.h>

#include <iostream>

using namespace adiscope;
using namespace gr;

cancel_dc_offset_block::cancel_dc_offset_block(size_t buffer_size, bool enabled):
	hier_block2("DCOFFSET",
		    io_signature::make(1, 1, sizeof(float)),
		    io_signature::make(1, 1, sizeof(float))),
	d_enabled(enabled),
	d_buffer_size(buffer_size)
{
	_build_and_connect_blocks();
}

cancel_dc_offset_block::~cancel_dc_offset_block()
{
}

void cancel_dc_offset_block::set_enabled(bool enabled)
{
	if (d_enabled != enabled) {
		d_enabled = enabled;

		_build_and_connect_blocks();
	}
}

void cancel_dc_offset_block::_build_and_connect_blocks()
{
	// Remove all connections
	hier_block2::disconnect_all();

	if (d_enabled) {
		//////////////////////////////////////////////////////////////////
		// this -->average -->skiphead -->repeat -->|                   //
		//   |                                      | -->sub_ff -->this //
		//    ------------------------------------->|                   //
		//////////////////////////////////////////////////////////////////

		auto avg = blocks::moving_average_ff::make(d_buffer_size,
							   1.0 / (float)d_buffer_size, d_buffer_size);
		auto sub_ff = blocks::sub_ff::make();
		auto skip_head = blocks::skiphead::make(sizeof(float), d_buffer_size - 1);
		auto repeat = blocks::repeat::make(sizeof(float), d_buffer_size);

		hier_block2::connect(this->self(), 0, sub_ff, 0);
		hier_block2::connect(this->self(), 0, avg, 0);
		hier_block2::connect(avg, 0, skip_head, 0);
		hier_block2::connect(skip_head, 0, repeat, 0);
		hier_block2::connect(repeat, 0, sub_ff, 1);
		hier_block2::connect(sub_ff, 0, this->self(), 0);
	} else {
		// Copy input to output port
		auto copy = blocks::copy::make(sizeof(float));
		hier_block2::connect(this->self(), 0, copy, 0);
		hier_block2::connect(copy, 0, this->self(), 0);
	}
}
