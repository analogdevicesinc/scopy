/* -*- c++ -*- */
/*
 * Copyright 2019 Analog Devices Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef OVERSHOOT_FILTER_HPP
#define OVERSHOOT_FILTER_HPP

#include "frequency_compensation_filter.h"
#include <inttypes.h>

using namespace gr;

namespace adiscope {
    class frequency_compensation_filter_impl : public frequency_compensation_filter
    {
    private:
	    bool enable;
	    float TC, gain;
	    float sample_rate;
	    int iterations;

    public:
	typedef boost::shared_ptr<frequency_compensation_filter> sptr;
	frequency_compensation_filter_impl(bool enable, float TC, float gain,
			      float sample_rate);
        int work(int noutput_items,
             gr_vector_const_void_star &input_items,
             gr_vector_void_star &output_items);

	void set_enable(bool en);
	bool get_enable() override;
	void set_TC(float TC);
	float get_TC() override;
	void set_gain(float gain);
	float get_gain() override;
	void set_sample_rate(float sample_rate);
    };
}
#endif
