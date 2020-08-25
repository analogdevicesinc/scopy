#ifndef LOGIC_ANALYZER_SINK_H
#define LOGIC_ANALYZER_SINK_H

#include <gnuradio/sync_block.h>

#include "logicanalyzer/logic_analyzer.h"

namespace adiscope {
class scope_sink_f;
}

class logic_analyzer_sink : virtual public gr::sync_block
{
public:
	typedef boost::shared_ptr<logic_analyzer_sink> sptr;

	static sptr make(adiscope::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	virtual void clean_buffers() = 0;
	virtual void set_nsamps(int newsize) = 0;
	virtual void set_can_plot(bool canPlot) = 0;
	virtual void set_scope_sink(boost::shared_ptr<adiscope::scope_sink_f> scope_sink) = 0;
};

#endif // LOGIC_ANALYZER_SINK_H
