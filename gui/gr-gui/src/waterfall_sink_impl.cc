/* -*- c++ -*- */
/*
 * Copyright 2012,2014-2015 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
/*
 * Copyright (c) 2022 Analog Devices Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "spectrumUpdateEvents.h"
#include "trigger_mode.h"
#include "waterfall_sink_impl.h"

#include <gnuradio/io_signature.h>
#include <gnuradio/prefs.h>

#include <cstring>

namespace scopy {

waterfall_sink::sptr waterfall_sink::make(int fftsize, std::vector<float> win, double fc, double bw,
					  const std::string &name, int nconnections, WaterfallDisplayPlot *plot,
					  bool fft_shift)
{
	return gnuradio::get_initial_sptr(
		new waterfall_sink_impl(fftsize, win, fc, bw, name, nconnections, plot, fft_shift));
}

waterfall_sink_impl::waterfall_sink_impl(int fftsize, std::vector<float> win, double fc, double bw,
					 const std::string &name, int nconnections, WaterfallDisplayPlot *plot,
					 bool fft_shift)
	: sync_block("waterfall_sink", gr::io_signature::make(0, nconnections, sizeof(gr_complex)),
		     gr::io_signature::make(0, 0, 0))
	, d_fftsize(0)
	, d_fft_shift(fftsize)
	, d_fftavg(1)
	, d_window(win)
	, d_center_freq(fc)
	, d_bandwidth(bw)
	, d_name(name)
	, d_nconnections(nconnections)
	, d_nrows(200)
	, d_port(pmt::mp("freq"))
	, d_port_bw(pmt::mp("bw"))
	, d_fft(std::make_unique<gr::fft::fft_complex_fwd>(fftsize))
	, d_fbuffers(d_nconnections)
	, d_buffers(d_nconnections)
	, d_fbuf(fftsize)
	, d_main_gui(plot)
	, do_shift(fft_shift)
	, d_tags(std::vector<std::vector<gr::tag_t>>(d_nconnections))
	, d_displayOneBuffer(true)
	, d_cleanBuffers(true)
	, d_trigger_mode(TRIG_MODE_TAG)
{
	resize_bufs(fftsize);
	d_fftsize = fftsize;
	initialize();
}

waterfall_sink_impl::~waterfall_sink_impl() { delete d_main_gui; }

bool waterfall_sink_impl::check_topology(int ninputs, int noutputs) { return ninputs == d_nconnections; }

// requests buffers with a max size of 8191 (useful at large fft sizes)
void waterfall_sink_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
{
	unsigned int ninputs = ninput_items_required.size();
	for(unsigned int i = 0; i < ninputs; i++) {
		ninput_items_required[i] = std::min(d_fftsize, 8191);
	}
}

void waterfall_sink_impl::initialize()
{
	set_fft_size(d_fftsize);
	set_frequency_range(d_center_freq, d_bandwidth);

	// initialize update time to 10 times a second
	set_update_time(0.1);
}

QWidget *waterfall_sink_impl::qwidget() { return d_main_gui; }

void waterfall_sink_impl::clear_data() { d_main_gui->clearData(); }

void waterfall_sink_impl::set_fft_size(const int fftsize)
{
	d_main_gui->resetAvgAcquisitionTime();
	clear_data();

	resize_bufs(fftsize);
	d_fftsize = fftsize;
	fftresize();
}

void waterfall_sink_impl::set_fft_window(const std::vector<float> window) { d_window = window; }

int waterfall_sink_impl::fft_size() const { return d_fftsize; }

float waterfall_sink_impl::fft_average() const { return d_fftavg; }

void waterfall_sink_impl::set_frequency_range(const double centerfreq, const double bandwidth)
{
	clear_data();
	d_center_freq = centerfreq;
	d_bandwidth = bandwidth;
	d_main_gui->setFrequencyRange(d_center_freq, d_bandwidth, 1., "Hz");
}

void waterfall_sink_impl::set_intensity_range(const double min, const double max)
{
	d_main_gui->setIntensityRange(min, max);
}

void waterfall_sink_impl::set_update_time(double t)
{
	// convert update time to ticks
	gr::high_res_timer_type tps = gr::high_res_timer_tps();
	d_update_time = t * tps;
	d_main_gui->setUpdateTime(t);
	d_last_time = 0;
}

void waterfall_sink_impl::set_line_label(unsigned int which, const std::string &label)
{
	d_main_gui->setLineLabel(which, label.c_str());
}

void waterfall_sink_impl::set_line_alpha(unsigned int which, double alpha)
{
	d_main_gui->setAlpha(which, (int)(255.0 * alpha));
}

void waterfall_sink_impl::set_size(int width, int height) { d_main_gui->resize(QSize(width, height)); }

void waterfall_sink_impl::set_plot_pos_half(bool half)
{
	clear_data();
	d_main_gui->setPlotPosHalf(half);
}

double waterfall_sink_impl::line_alpha(unsigned int which) { return (double)(d_main_gui->getAlpha(which)) / 255.0; }

void waterfall_sink_impl::auto_scale() { d_main_gui->autoScale(); }

double waterfall_sink_impl::min_intensity(unsigned int which) { return d_main_gui->getMinIntensity(which); }

double waterfall_sink_impl::max_intensity(unsigned int which) { return d_main_gui->getMaxIntensity(which); }

void waterfall_sink_impl::disable_legend() { d_main_gui->disableLegend(); }

void waterfall_sink_impl::fft(float *data_out, gr_complex *data_in, int size)
{
	// float to complex conversion
	if(!d_window.empty()) {
		volk_32fc_32f_multiply_32fc(d_fft->get_inbuf(), data_in, &d_window.front(), size);
	} else {
		memcpy(d_fft->get_inbuf(), data_in, sizeof(gr_complex) * size);
	}

	d_fft->execute(); // compute the fft

	volk_32fc_s32f_x2_power_spectral_density_32f(data_out, d_fft->get_outbuf(), size, 1.0, size);

	if(do_shift) {
		d_fft_shift.shift(data_out, size);
	}
}

void waterfall_sink_impl::resize_bufs(int size)
{
	if(size != fft_size()) {
		gr::thread::scoped_lock lock(d_setlock);

		// Set new size and reset buffer index
		// (throws away any currently held data, but who cares?)
		d_fftsize = size;

		// Resize buffers and replace data
		for(int n = 0; n < d_nconnections; n++) {
			volk_free(d_buffers[n]);
			d_buffers[n] = (double *)volk_malloc(d_fftsize * sizeof(double), volk_get_alignment());
			memset(d_buffers[n], 0, d_fftsize * sizeof(double));

			volk_free(d_fbuffers[n]);
			d_fbuffers[n] = (gr_complex *)volk_malloc(d_fftsize * sizeof(gr_complex), volk_get_alignment());
			memset(d_fbuffers[n], 0, d_fftsize * sizeof(gr_complex));
		}

		_reset();
		d_cleanBuffers = true;
	}
}

void waterfall_sink_impl::fftresize()
{
	gr::thread::scoped_lock lock(d_setlock);

	// Set new fft size and reset buffer index
	d_index = 0;

	// Reset FFTW plan for new size
	d_fft = std::make_unique<fft::fft_complex_fwd>(d_fftsize);

	d_fft_shift.resize(d_fftsize);

	d_fbuf.clear();
	d_fbuf.resize(d_fftsize);

	d_last_time = 0;
}

void waterfall_sink_impl::set_trigger_mode(trigger_mode mode, int channel, const std::string &tag_key)
{
	gr::thread::scoped_lock lock(d_setlock);
	d_trigger_mode = mode;
	d_trigger_channel = channel;
	d_trigger_tag_key = pmt::intern(tag_key);
	d_triggered = false;

	_reset();
}

void waterfall_sink_impl::set_time_per_fft(double t) { d_main_gui->setUpdateTime(t); }

void waterfall_sink_impl::_test_trigger_tags(int nitems)
{
	int trigger_index;

	uint64_t nr = nitems_read(d_trigger_channel);
	std::vector<gr::tag_t> tags;
	get_tags_in_range(tags, d_trigger_channel, nr, nr + nitems + 1, d_trigger_tag_key);

	if(tags.size() > 0) {
		d_triggered = true;
		trigger_index = tags[0].offset - nr;
		d_start = d_index + trigger_index;
		d_end = d_start + fft_size();
		_adjust_tags(-d_start);
	}
}

void waterfall_sink_impl::_adjust_tags(int adj)
{
	for(size_t n = 0; n < d_tags.size(); n++) {
		for(size_t t = 0; t < d_tags[n].size(); t++) {
			d_tags[n][t].offset += adj;
		}
	}
}

void waterfall_sink_impl::reset()
{
	gr::thread::scoped_lock lock(d_setlock);
	_reset();
}

void waterfall_sink_impl::set_displayOneBuffer(bool val) { d_displayOneBuffer = val; }

void waterfall_sink_impl::_reset()
{
	int n;

	for(n = 0; n < d_nconnections; n++) {
		d_tags[n].clear();
	}

	// Reset the start and end indices.
	d_start = 0;
	d_index = 0;
	d_end = fft_size();

	// Reset the trigger. If in free running mode, ignore the
	// trigger delay and always set trigger to true.
	if(d_trigger_mode == TRIG_MODE_FREE) {
		d_index = 0;
		d_triggered = true;
	} else {
		d_triggered = false;
	}
}

int waterfall_sink_impl::work(int noutput_items, gr_vector_const_void_star &input_items,
			      gr_vector_void_star &output_items)
{
	int n = 0, idx = 0;
	const gr_complex *in;

	gr::thread::scoped_lock lock(d_setlock);

	if(!d_displayOneBuffer && !d_cleanBuffers) {
		return 0;
	}
	int nfill = d_end - d_index;		     // how much room left in buffers
	int nitems = std::min(noutput_items, nfill); // num items we can put in buffers
	int nItemsToSend = 0;

	// If tag trigger, look for the trigger
	if((d_trigger_mode != TRIG_MODE_FREE) && !d_triggered) {
		// trigger off a tag key (first one found)
		if(d_trigger_mode == TRIG_MODE_TAG) {
			_test_trigger_tags(nitems);
		}
	}

	// Copy data into the buffers.
	for(n = 0; n < d_nconnections; n++) {
		in = (const gr_complex *)input_items[idx];

		memcpy(d_fbuffers[n] + d_index, &in[0], sizeof(gr_complex) * nitems);
		uint64_t nr = nitems_read(idx);
		std::vector<gr::tag_t> tags;
		get_tags_in_range(tags, idx, nr, nr + nitems + 1);

		for(size_t t = 0; t < tags.size(); t++) {
			tags[t].offset = tags[t].offset - nr + (d_index - d_start - 1);
		}
		d_tags[idx].insert(d_tags[idx].end(), tags.begin(), tags.end());
		idx++;
	}
	d_index += nitems;

	// If we've have a full d_size of items in the buffers, plot.
	if((d_end != 0 && !d_displayOneBuffer) ||
	   ((d_triggered) && (d_index == d_end) && d_end != 0 && d_displayOneBuffer)) {
		// Copy data to be plotted to start of buffers.
		for(n = 0; n < d_nconnections; n++) {
			if(!d_displayOneBuffer) {
				nItemsToSend = d_index;
				if(nItemsToSend >= fft_size()) {
					nItemsToSend = fft_size();
					d_cleanBuffers = false;
				}
			} else {
				nItemsToSend = fft_size();
			}

			fft(d_fbuf.data(), d_fbuffers[n], fft_size());
			for(int x = 0; x < fft_size(); x++) {
				d_buffers[n][x] = (double)((1.0 - d_fftavg) * d_buffers[n][x] + (d_fftavg)*d_fbuf[x]);
			}
		}

		// Plot if we are able to update
		if((gr::high_res_timer_now() - d_last_time > d_update_time) || !d_cleanBuffers) {
			d_last_time = gr::high_res_timer_now();
			d_qApplication->postEvent(d_main_gui,
						  new WaterfallUpdateEvent(d_buffers, nItemsToSend,
									   gr::high_res_timer_now() - d_last_time));
		}

		// We've plotting, so reset the state
		if(d_displayOneBuffer) {
			_reset();
		}
	}

	if(d_displayOneBuffer && d_index == d_end) {
		_reset();
	}
	return nitems;
}
} /* namespace scopy */
