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

#include "frequency_compensation_filter_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/char_to_short.h>

using namespace gr;

namespace adiscope {
frequency_compensation_filter_impl::frequency_compensation_filter_impl(
	bool enable, float TC, float gain,
	float sample_rate)

	: sync_block("overshoot_filter",
		     io_signature::make(1,1, sizeof(short)),
		     io_signature::make(1, 1, sizeof(short))),
	  sample_rate(sample_rate), high_gain(false)

{
	for (auto i=0; i < 2; i++) {
		config[i].enable = enable;
		config[i].TC = TC;
		config[i].gain = gain;
	}

	set_history(1);
}

frequency_compensation_filter::sptr
frequency_compensation_filter::make(bool enable, float TC, float gain,
				    float sample_rate)
{
	return gnuradio::get_initial_sptr
	       (new frequency_compensation_filter_impl(enable, TC, gain,
			       sample_rate));
}

int
frequency_compensation_filter_impl::work(int noutput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items)
{
	const short *in = (const short *) input_items[0];
	float *out_f = new float[noutput_items];
	short *out = (short *) output_items[0];

	float delta = 1.0 / sample_rate;
	float TC1 = config[high_gain].TC * float(1.0E-6);
	float Alpha = TC1/(TC1+delta);
	out_f[0]=(in[1]-in[0]);
	int i = 1;

	if (config[high_gain].enable) {
		for (i = 1; i < noutput_items; i++) {
			out_f[i]=(Alpha*(out_f[i-1]+(float)(in[i]-in[i-1])));
		}

		for (i = 0; i < noutput_items; i++) {
			out[i]=in[i]+(short)((out_f[i])*config[high_gain].gain);
		}

	} else {
		memcpy(out,in,noutput_items*sizeof(short));
	}

	delete []out_f;
	return noutput_items;
}

void frequency_compensation_filter_impl::set_enable(bool en, int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	this->config[gain_mode].enable = en;
}

bool frequency_compensation_filter_impl::get_enable(int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	return this->config[gain_mode].enable;
}

void frequency_compensation_filter_impl::set_TC(float TC, int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	this->config[gain_mode].TC = TC;
}

float frequency_compensation_filter_impl::get_TC(int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	return this->config[gain_mode].TC;
}

void frequency_compensation_filter_impl::set_filter_gain(float gain,
		int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	this->config[gain_mode].gain = gain;
}

float frequency_compensation_filter_impl::get_filter_gain(int gain_mode)
{
	if (gain_mode == 2) {
		gain_mode = high_gain;
	}

	return this->config[gain_mode].gain;
}

void frequency_compensation_filter_impl::set_sample_rate(float sample_rate)
{
	this->sample_rate = sample_rate;
}

bool frequency_compensation_filter_impl::get_high_gain()
{
	return this->high_gain;
}
void frequency_compensation_filter_impl::set_high_gain(bool en)
{
	this->high_gain=en;
}
}
