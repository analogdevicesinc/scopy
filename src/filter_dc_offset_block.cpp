#include "filter_dc_offset_block.h"

#include <volk/volk.h>
#include <iostream>

using namespace adiscope;

filter_dc_offset_block::filter_dc_offset_block(unsigned int buffer_size):
	gr::sync_block("dc_offset_filter_block",
		       gr::io_signature::make(1, 1, buffer_size * sizeof(float)),
		       gr::io_signature::make(1, 1, buffer_size * sizeof(float))),
	d_buffer_size(buffer_size)
{
}

filter_dc_offset_block::~filter_dc_offset_block()
{
}

int filter_dc_offset_block::work(int noutput_items,
				 gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	float sum;

	const float* in = static_cast<const float *>(input_items[0]);
	float* out = static_cast<float *>(output_items[0]);

	volk_32f_accumulator_s32f(&sum, in, d_buffer_size);

	float avg = (sum / (float)d_buffer_size);

	for (unsigned int i = 0 ; i < d_buffer_size; ++i) {
		out[i] = in[i] - avg;
	}

	std::cout << "Number of items: " << d_buffer_size << std::endl;
	std::cout << "Sum of items: " << sum << std::endl;
	std::cout << "Average of items: " << avg << std::endl;

	return noutput_items;
}
