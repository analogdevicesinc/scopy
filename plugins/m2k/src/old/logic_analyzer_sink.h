/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LOGIC_ANALYZER_SINK_H
#define LOGIC_ANALYZER_SINK_H

#include "logicanalyzer/logic_analyzer.h"

#include <gnuradio/sync_block.h>

class logic_analyzer_sink : virtual public gr::sync_block
{
public:
	typedef std::shared_ptr<logic_analyzer_sink> sptr;

	static sptr make(scopy::m2k::logic::LogicAnalyzer *logicAnalyzer, int bufferSize);

	virtual void clean_buffers() = 0;
	virtual void set_nsamps(int newsize) = 0;
};

#endif // LOGIC_ANALYZER_SINK_H
