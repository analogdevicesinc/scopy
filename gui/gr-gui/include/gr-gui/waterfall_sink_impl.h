/* -*- c++ -*- */
/*
 * Copyright 2012,2015,2019 Free Software Foundation, Inc.
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

#ifndef INCLUDED_QTGUI_WATERFALL_SINK_IMPL_H
#define INCLUDED_QTGUI_WATERFALL_SINK_IMPL_H

#include "WaterfallDisplayPlot.h"
#include "waterfall_sink.h"

#include <gnuradio/fft/fft.h>
#include <gnuradio/fft/fft_shift.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/high_res_timer.h>

#include <trigger_mode.h>

using namespace gr;
namespace scopy {

class waterfall_sink_impl : public waterfall_sink
{
private:
	void forecast(int noutput_items, gr_vector_int &ninput_items_required) override;

	void initialize();

	int d_fftsize;
	gr::fft::fft_shift<float> d_fft_shift;
	float d_fftavg;
	std::vector<float> d_window;
	double d_center_freq;
	double d_bandwidth;
	const std::string d_name;
	int d_nconnections;
	int d_nrows;

	const pmt::pmt_t d_port;
	const pmt::pmt_t d_port_bw;

	// Perform fftshift operation;
	// this is usually desired when plotting
	std::unique_ptr<gr::fft::fft_complex_fwd> d_fft;

	int d_index, d_start, d_end;

	std::vector<gr_complex *> d_fbuffers;
	std::vector<double *> d_buffers;

	double *d_pdu_magbuf;
	volk::vector<float> d_fbuf;

	// Required now for Qt; argc must be greater than 0 and argv
	// must have at least one valid character. Must be valid through
	// life of the qApplication:
	// http://harmattan-dev.nokia.com/docs/library/html/qt4/qapplication.html
	char d_zero = 0;
	int d_argc = 1;
	char *d_argv = &d_zero;
	WaterfallDisplayPlot *d_main_gui = nullptr;

	gr::high_res_timer_type d_update_time;
	gr::high_res_timer_type d_last_time;
	bool do_shift;

	void fftresize();
	void resize_bufs(int size);
	void check_clicked();
	void fft(float *data_out, gr_complex *data_in, int size);
	void _test_trigger_tags(int nitems);
	void _adjust_tags(int adj);
	void _reset();
	void set_displayOneBuffer(bool val);
	void clean_buffers();

	std::vector<std::vector<gr::tag_t>> d_tags;

	bool d_displayOneBuffer;
	bool d_cleanBuffers;

	trigger_mode d_trigger_mode;
	bool d_triggered;
	int d_trigger_channel;
	pmt::pmt_t d_trigger_tag_key;

public:
	waterfall_sink_impl(int size, std::vector<float> win, double fc, double bw, const std::string &name,
			    int nconnections, WaterfallDisplayPlot *plot = NULL, bool fft_shift = false);
	~waterfall_sink_impl();

	void reset() override;

	bool check_topology(int ninputs, int noutputs) override;

	QWidget *qwidget() override;

	void clear_data() override;

	void set_fft_size(const int fftsize) override;
	int fft_size() const override;
	float fft_average() const override;
	void set_fft_window(const std::vector<float> window) override;

	void set_frequency_range(const double centerfreq, const double bandwidth) override;
	void set_intensity_range(const double min, const double max) override;

	void set_update_time(double t) override;
	void set_time_per_fft(double t) override;
	void set_line_label(unsigned int which, const std::string &label) override;
	void set_line_alpha(unsigned int which, double alpha) override;
	void set_plot_pos_half(bool half) override;

	double line_alpha(unsigned int which) override;

	void set_size(int width, int height) override;

	void auto_scale() override;
	double min_intensity(unsigned int which) override;
	double max_intensity(unsigned int which) override;

	void set_trigger_mode(trigger_mode mode, int channel, const std::string &tag_key = "") override;

	void disable_legend() override;

	int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) override;
};

} /* namespace scopy */

#endif /* INCLUDED_QTGUI_waterfall_sink_IMPL_H */
