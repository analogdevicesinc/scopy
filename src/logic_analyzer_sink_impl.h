#ifndef LOGIC_ANALYZER_SINK_IMPL_H
#define LOGIC_ANALYZER_SINK_IMPL_H

#include "logic_analyzer_sink.h"
#include "scope_sink_f.h"

#include <atomic>

class logic_analyzer_sink_impl : public logic_analyzer_sink
{
public:
	logic_analyzer_sink_impl(adiscope::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	int work(int noutput_items,
	         gr_vector_const_void_star &input_items,
	         gr_vector_void_star &output_items);

	void clean_buffers();
	void set_nsamps(int newsize);
	void set_can_plot(bool canPlot);
	void set_scope_sink(adiscope::scope_sink_f::sptr scope_sink);

private:
	void _adjust_tags(int adj);
	void _test_trigger_tags(int nitems);
	void _reset();

private:
	adiscope::logic::LogicAnalyzer *d_logic_analyzer;
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

	std::atomic<bool> d_can_plot;
	adiscope::scope_sink_f::sptr d_scope_sink;
};

#endif // LOGIC_ANALYZER_SINK_IMPL_H
