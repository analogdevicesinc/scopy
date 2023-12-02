/* -*- c++ -*- */
/*
 * Copyright 2012,2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#ifndef TIME_SINK_F_H
#define TIME_SINK_F_H

#ifdef ENABLE_PYTHON
#include <Python.h>
#endif

#include "scopy-gr-util_export.h"

#include <gnuradio/sync_block.h>

#include <QString>

namespace scopy {

typedef struct
{
	QString str;
	int offset;
} PlotTag_t;

class SCOPY_GR_UTIL_EXPORT time_sink_f : virtual public gr::sync_block
{
public:
	// scopy::time_sink_f::sptr
	typedef std::shared_ptr<time_sink_f> sptr;

	static sptr make(int size, float sampleRate, const std::string &name, int nconnections);
	virtual std::string name() const = 0;
	virtual void updateData() = 0;
	virtual const std::vector<float> &time() const = 0;
	virtual const std::vector<float> &freq() const = 0;
	virtual const std::vector<std::vector<float>> &data() const = 0;
	virtual const std::vector<std::vector<PlotTag_t>> &tags() const = 0;
	virtual void setRollingMode(bool) = 0;
	virtual bool rollingMode() = 0;
	virtual void setSingleShot(bool) = 0;
	virtual bool singleShot() = 0;
	virtual bool finishedAcquisition() = 0;
	virtual bool computeTags() = 0;
	virtual float freqOffset() = 0;
	virtual void setFreqOffset(float) = 0;
	virtual void setComputeTags(bool newComputeTags) = 0;
	virtual bool fftComplex() = 0;
	virtual void setFftComplex(bool) = 0;
};

} /* namespace scopy */

#endif /* M2K_time_sink_f_H */
