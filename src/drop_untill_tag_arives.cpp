#include "drop_untill_tag_arives.h"

drop_untill_tag_arives::drop_untill_tag_arives(size_t itemsize)
	: block("drop_untill_tags_arive", gr::io_signature::make(1, 2, itemsize),
		gr::io_signature::make(1, 2, itemsize))
{
	/* By default there is no reason to further propagate tags */
	set_tag_propagation_policy(TPP_DONT);

	d_copied_items.resize(2); // BAD
	d_buffer_size.resize(2);  // BAD

	for (size_t i = 0; i < d_copied_items.size(); ++i) {
		d_copied_items[i] = 0;
	}
}

drop_untill_tag_arives::~drop_untill_tag_arives() {}

void drop_untill_tag_arives::reset()
{
	for (size_t i = 0; i < d_copied_items.size(); ++i) {
		d_copied_items[i] = 0;
	}
}

void drop_untill_tag_arives::set_buffer_size(unsigned int buffer_size)
{
	for (size_t i = 0; i < d_buffer_size.size(); ++i) {
		d_buffer_size[i] = buffer_size;
	}
}

int drop_untill_tag_arives::general_work(int noutput_items, gr_vector_int& ninput_items_,
					 gr_vector_const_void_star& input_items, gr_vector_void_star& output_items)
{

	int processed_items = 0;

	for (int i = 0; i < ninput_items_.size(); ++i) {
		//		std::cout << "Channel : " << i << std::endl;

		const float* in = static_cast<const float*>(input_items[i]);
		float* out = static_cast<float*>(output_items[i]);

		int ninput_items = (int)std::min((int)d_buffer_size[i] - (int)d_copied_items[i], noutput_items);

		processed_items = ninput_items;

		if (d_copied_items[i] >= d_buffer_size[i]) {
			bool stopFlowgraph = true;
			for (size_t j = 0; j < d_copied_items.size(); ++j) {
				if (d_copied_items[j] < d_buffer_size[j]) {
					stopFlowgraph = false;
				}
			}
			if (stopFlowgraph) {
				consume(i, ninput_items);
				return -1;
			} else {
				consume(i, ninput_items);
				continue;
			}
		}

		//		// Get tags in range and print each tag key
		// value pairs + index in buffer
		// std::vector<gr::tag_t> tags; 		uint64_t nr =
		// nitems_read(i); 		get_tags_in_range(tags, i, nr,
		// nr
		// + noutput_items + 1);

		//		if (tags.size()) {
		//			std::cout << "No of tags: " <<
		// tags.size()
		//<< " value: " << tags[0].offset
		//				  << " Interval: " << nr << " -
		//"
		//<< nr + noutput_items + 1 << std::endl; 		} else {
		// std::cout << " Interval: " << nr << " - " << nr +
		// noutput_items + 1 << std::endl;
		//		}

		// Copy in to out for now untill counter for this channel
		// reached
		memcpy(out, in, sizeof(float) * ninput_items);
		d_copied_items[i] += ninput_items;

		consume(i, ninput_items);
	}

	return processed_items;
}
