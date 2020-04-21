#ifndef DROP_UNTILL_TAG_ARIVES_H
#define DROP_UNTILL_TAG_ARIVES_H

#include <gnuradio/block.h>

class drop_untill_tag_arives : public gr::block {
public:
	explicit drop_untill_tag_arives(size_t itemsize);
	~drop_untill_tag_arives();

	void reset();
	void set_buffer_size(unsigned int buffer_size);

	int general_work(int noutput_items, gr_vector_int &ninput_items_, gr_vector_const_void_star &input_items,
			 gr_vector_void_star &output_items);

private:
	std::vector<unsigned int> d_buffer_size;
	std::vector<unsigned int> d_copied_items;
};

#endif // DROP_UNTILL_TAG_ARIVES_H
