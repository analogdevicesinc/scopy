/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIXED_SIGNAL_SINK_H
#define MIXED_SIGNAL_SINK_H

#include <gnuradio/sync_block.h>

#include "logicanalyzer/logic_analyzer.h"
#include "TimeDomainDisplayPlot.h"

class mixed_signal_sink : virtual public gr::sync_block
{
public:
	typedef std::shared_ptr<mixed_signal_sink> sptr;

	static sptr make(adiscope::logic::LogicAnalyzer *logicAnalyzer,
	                 adiscope::TimeDomainDisplayPlot *oscPlot,
	                 int bufferSize);

	virtual void clean_buffers() = 0;
	virtual void set_nsamps(int newsize) = 0;
	virtual void set_displayOneBuffer(bool display) = 0;
	virtual void set_update_time(double t) = 0;
};

#endif // MIXED_SIGNAL_SINK_H
