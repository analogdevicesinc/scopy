#include "filter_dc_offset_block.h"

#include <volk/volk.h>
#include <iostream>

using namespace adiscope;

filter_dc_offset_block::filter_dc_offset_block(unsigned int buffer_size, bool enabled):
	gr::sync_block("dc_offset_filter_block",
		       gr::io_signature::make(1, 1, buffer_size * sizeof(float)),
		       gr::io_signature::make(1, 1, buffer_size * sizeof(float))),
	d_buffer_size(buffer_size), d_dc_offset(0.0), d_enabled(enabled)
{
}

filter_dc_offset_block::~filter_dc_offset_block()
{
}

float filter_dc_offset_block::get_dc_offset() const
{
	return d_dc_offset;
}

int filter_dc_offset_block::work(int noutput_items,
				 gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	const float* in = static_cast<const float *>(input_items[0]);
	float* out = static_cast<float *>(output_items[0]);

	float sum = 0.0;
	volk_32f_accumulator_s32f(&sum, in, d_buffer_size);

	float avg = (sum / (float)d_buffer_size);

	if (d_enabled) {
		for (unsigned int i = 0 ; i < d_buffer_size; ++i) {
			out[i] = in[i] - avg;
		}
	}  else {
		memcpy(out, in, d_buffer_size * sizeof(float));
	}

	// Save dc offset
	d_dc_offset = avg;

	return noutput_items;
}
