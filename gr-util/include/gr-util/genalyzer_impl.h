/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef INCLUDED_GENALYZER_FFT_VCC_IMPL_H
#define INCLUDED_GENALYZER_FFT_VCC_IMPL_H

#include "genalyzer.h"
#include <cgenalyzer_simplified_beta.h>
#include <mutex>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT genalyzer_fft_vii_impl : public genalyzer_fft_vii
{
private:
	size_t d_npts;
	int d_qres;
	size_t d_navg;
	size_t d_nfft;
	GnWindow d_win;
	double d_sample_rate;
	gn_config d_config;
	double *d_fft_out;
	int32_t *d_qwfi;
	int32_t *d_qwfq;

	// Moving average buffer management - circular buffer approach
	int32_t **d_frame_buffers_i;	    // Array of pointers to I channel frame buffers
	int32_t **d_frame_buffers_q;	    // Array of pointers to Q channel frame buffers
	size_t d_current_frame_index;	    // Current frame index in circular buffer
	size_t d_frames_filled;		    // Number of frames filled (0 to d_navg)
	size_t d_allocated_frames;	    // Number of frame buffers actually allocated
	bool d_do_shift;		    // Whether to apply FFT shift
	GenalyzerConfig d_genalyzer_config; // Complete genalyzer configuration
	char *d_fa_key = nullptr;	    // Fourier analysis configuration key

	gn_analysis_results *d_analysis;

	// Protect against library-level threading issues
	// genalyzer uses fftw3 library which cannot be used simultaniously from
	// different threads from different gr::sync_block instances
	static std::mutex s_genalyzer_mutex;

	// Instance-specific mutex to protect buffer operations during navg changes
	std::mutex d_buffer_mutex;

	void cleanup_buffers();
	void cleanup_frame_buffers(); // Clean only circular buffer frames
	void allocate_buffers();

	// Helper functions for work()
	void store_frame_to_circular_buffer(const int32_t *in_i_vec, const int32_t *in_q_vec);
	size_t prepare_frames_for_processing(size_t &current_npts);
	int perform_fft_and_convert_to_db(size_t frames_to_process, size_t current_npts, double *shifted_output,
					  double *db_output, float *out_vec);
	void perform_genalyzer_analysis();

	// Analysis result helpers
	void clear_analysis_results();
	void set_analysis_results(size_t results_size, char **rkeys, double *rvalues);
	void free_analysis_keys(char **rkeys, size_t count);

	// auto analysis
	void perform_auto_analysis();
	int configure_auto_analysis();

	// fixed tone analysis
	int configure_fixed_tone_analysis();
	void perform_fixed_tone_analysis();

	void cleanup_fa_config();
	void cleanup_auto_config();

public:
	genalyzer_fft_vii_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate,
			       bool do_shift = true);
	~genalyzer_fft_vii_impl();

	void set_sample_rate(double sample_rate) override;
	double sample_rate() const override;
	void set_window(GnWindow win) override;
	int window() const override;
	int navg() const override;
	void set_config(const GenalyzerConfig &config) override;
	GenalyzerConfig get_config() const override;

	bool start() override;
	bool stop() override;
	int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) override;

	gn_analysis_results *getGnAnalysis() override;
};

} // namespace scopy::grutil

#endif /* INCLUDED_GENALYZER_FFT_VCC_IMPL_H */
