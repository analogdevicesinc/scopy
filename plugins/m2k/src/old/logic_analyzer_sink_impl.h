#ifndef LOGIC_ANALYZER_SINK_IMPL_H
#define LOGIC_ANALYZER_SINK_IMPL_H

#include "logic_analyzer_sink.h"

class logic_analyzer_sink_impl : public logic_analyzer_sink
{
public:
	logic_analyzer_sink_impl(scopy::m2k::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	int work(int noutput_items,
	         gr_vector_const_void_star &input_items,
	         gr_vector_void_star &output_items);

	void clean_buffers();
	void set_nsamps(int newsize);

private:
	void _adjust_tags(int adj);
	void _test_trigger_tags(int nitems);
	void _reset();

private:
	scopy::m2k::logic::LogicAnalyzer *d_logic_analyzer;
	uint16_t *d_buffer;
	uint16_t *d_buffer_temp;
	int d_size;
	int d_buffer_size;
	int d_index;
	int d_end;
	int d_start;

	std::vector< std::vector<gr::tag_t> > d_tags;
	pmt::pmt_t d_trigger_tag_key;
	bool d_triggered;

	gr::high_res_timer_type d_update_time;
	gr::high_res_timer_type d_last_time;
};

#endif // LOGIC_ANALYZER_SINK_IMPL_H
