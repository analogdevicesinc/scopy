#ifndef LOGIC_ANALYZER_SINK_H
#define LOGIC_ANALYZER_SINK_H

#include <gnuradio/sync_block.h>

#include "logicanalyzer/logic_analyzer.h"

class logic_analyzer_sink : virtual public gr::sync_block
{
public:
	typedef boost::shared_ptr<logic_analyzer_sink> sptr;

	static sptr make(adiscope::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	virtual void clean_buffers() = 0;
	virtual void set_nsamps(int newsize) = 0;
};

#endif // LOGIC_ANALYZER_SINK_H
