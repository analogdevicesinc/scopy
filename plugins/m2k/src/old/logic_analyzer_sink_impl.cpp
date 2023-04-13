#include "logic_analyzer_sink_impl.h"

#include <gnuradio/io_signature.h>

#include <volk/volk.h>

using namespace gr;

logic_analyzer_sink::sptr logic_analyzer_sink::make(scopy::m2k::logic::LogicAnalyzer *logicAnalyzer,
						    int bufferSize)
{
	return gnuradio::get_initial_sptr(
				new logic_analyzer_sink_impl(logicAnalyzer, bufferSize));
}

logic_analyzer_sink_impl::logic_analyzer_sink_impl(scopy::m2k::logic::LogicAnalyzer *logicAnalyzer,
						   int bufferSize)
	: sync_block("logic_analyzer_sync",
		     io_signature::make(1, 1, sizeof(uint16_t)),
		     io_signature::make(0, 0, 0))
	, d_logic_analyzer(logicAnalyzer)
	, d_buffer(nullptr)
	, d_buffer_temp(nullptr)
	, d_size(bufferSize)
	, d_buffer_size(2 * bufferSize)
	, d_index(0)
	, d_end(d_size)
	, d_tags(std::vector< std::vector<gr::tag_t> >(1))
	, d_trigger_tag_key(pmt::intern("buffer_start"))
	, d_triggered(false)
	, d_update_time(0.1 * gr::high_res_timer_tps())
	, d_last_time(0)
{

	d_buffer = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
	memset(d_buffer, 0, sizeof(uint16_t) * d_buffer_size);

	d_buffer_temp = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
	memset(d_buffer_temp, 0, sizeof(uint16_t) * d_buffer_size);

	const int alignment_multiple =
	  volk_get_alignment() / sizeof(uint16_t);
	set_alignment(std::max(1,alignment_multiple));
}

int logic_analyzer_sink_impl::work(int noutput_items,
				   gr_vector_const_void_star &input_items,
				   gr_vector_void_star &output_items)
{
	gr::thread::scoped_lock lock(d_setlock);

	// space left in buffer
	const int nfill = d_end - d_index;
	// num items we can put in the buffer
	const int nitems = std::min(noutput_items, nfill);

	// look for trigger tag
	_test_trigger_tags(nitems);

	// copy data into the buffers + tags in range
	const uint16_t *in = static_cast<const uint16_t *>(input_items[0]);
	memcpy(d_buffer_temp + d_index, in, sizeof(uint16_t) * nitems);

	uint64_t nr = nitems_read(0);
	std::vector<gr::tag_t> tags;
	get_tags_in_range(tags, 0, nr, nr + nitems + 1);
	for(size_t t = 0; t < tags.size(); t++) {
		tags[t].offset = tags[t].offset - nr + (d_index - d_start - 1);
	}
	d_tags[0].insert(d_tags[0].end(), tags.begin(), tags.end());
	// send data to logic analyzer

	d_index += nitems;

	if (d_triggered && (d_index == d_end) && d_end != 0) {
		if (gr::high_res_timer_now() - d_last_time > d_update_time) {
			d_last_time = gr::high_res_timer_now();
			d_logic_analyzer->setData(d_buffer_temp + d_start, d_size);
		}

		_reset();
	}

	if (d_index == d_end) {
		_reset();
	}

	return nitems;
}

void logic_analyzer_sink_impl::clean_buffers()
{
	gr::thread::scoped_lock lock(d_setlock);

	d_buffer_size = 2 * d_size;

	volk_free(d_buffer);
	d_buffer = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
	memset(d_buffer, 0, sizeof(uint16_t) * d_buffer_size);

	volk_free(d_buffer_temp);
	d_buffer_temp = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
	memset(d_buffer_temp, 0, sizeof(uint16_t) * d_buffer_size);

	_reset();
}

void logic_analyzer_sink_impl::set_nsamps(int newsize)
{
	if (newsize != d_size) {
		gr::thread::scoped_lock lock(d_setlock);

		// set new size
		d_size = newsize;
		d_buffer_size = 2 * d_size;

		// free old buffers
		volk_free(d_buffer);
		volk_free(d_buffer_temp);

		// create new buffers
		d_buffer = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
		memset(d_buffer, 0, sizeof(uint16_t) * d_buffer_size);
		d_buffer_temp = (uint16_t*)volk_malloc(d_buffer_size * sizeof(uint16_t), volk_get_alignment());
		memset(d_buffer_temp, 0, sizeof(uint16_t) * d_buffer_size);

		_reset();
	}
}

void logic_analyzer_sink_impl::_adjust_tags(int adj)
{
	for(size_t n = 0; n < d_tags.size(); n++) {
		for(size_t t = 0; t < d_tags[n].size(); t++) {
		d_tags[n][t].offset += adj;
		}
	}
}

void logic_analyzer_sink_impl::_test_trigger_tags(int nitems)
{
	int trigger_index;

	uint64_t nr = nitems_read(0);
	std::vector<gr::tag_t> tags;
	get_tags_in_range(tags, 0, nr, nr + nitems + 1, d_trigger_tag_key);
	if (tags.size() > 0) {
		d_triggered = true;
		trigger_index = tags[0].offset - nr;
		d_start = d_index + trigger_index;
		d_end = d_start + d_size;
		_adjust_tags(-d_start);
	}
}

void logic_analyzer_sink_impl::_reset()
{
	d_tags[0].clear();

	d_start = 0;
	d_index = 0;
	d_end = d_size;

	d_triggered = false;
}
