#ifndef FILTER_DC_OFFSET_BLOCK_H
#define FILTER_DC_OFFSET_BLOCK_H

#include <gnuradio/sync_block.h>

namespace adiscope {
class filter_dc_offset_block : public gr::sync_block
{
public:
	explicit filter_dc_offset_block(unsigned int buffer_size);
	~filter_dc_offset_block();

	int work(int noutput_items,
		 gr_vector_const_void_star &input_items,
		 gr_vector_void_star &output_items);

private:
	unsigned d_buffer_size;
};
}

#endif // FILTER_DC_OFFSET_BLOCK_H
