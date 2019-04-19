#include "cancel_dc_offset_block.h"

#include <gnuradio/blocks/copy.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>

#include <iostream>

using namespace adiscope;
using namespace gr;

cancel_dc_offset_block::cancel_dc_offset_block(size_t buffer_size, bool enabled):
	hier_block2("DCOFFSET",
		    io_signature::make(1, 1, sizeof(float)),
		    io_signature::make(1, 1, sizeof(float))),
	d_enabled(enabled),
	d_buffer_size(buffer_size),
	d_dc_offset(0.0)
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

void cancel_dc_offset_block::set_buffer_size(size_t buffer_size)
{
	if (d_buffer_size != buffer_size) {
		d_buffer_size = buffer_size;
		_build_and_connect_blocks();
	}
}

float cancel_dc_offset_block::get_dc_offset() const
{
	return d_cancel_dc->get_dc_offset();
}

void cancel_dc_offset_block::_build_and_connect_blocks()
{
	// Remove all connections
	hier_block2::disconnect_all();

	auto s2v = gr::blocks::stream_to_vector::make(sizeof(float), d_buffer_size);
	auto v2s = gr::blocks::vector_to_stream::make(sizeof(float), d_buffer_size);

	d_cancel_dc = gnuradio::get_initial_sptr(
				new filter_dc_offset_block(d_buffer_size, d_enabled));

	hier_block2::connect(this->self(), 0, s2v, 0);
	hier_block2::connect(s2v, 0, d_cancel_dc, 0);
	hier_block2::connect(d_cancel_dc, 0, v2s, 0);
	hier_block2::connect(v2s, 0, this->self(), 0);
}
