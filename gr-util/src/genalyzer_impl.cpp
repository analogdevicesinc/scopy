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

#include "genalyzer.h"
#include "genalyzer_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <cmath>
#include <cgenalyzer_simplified_beta.h>
#include <cgenalyzer.h>

using namespace scopy::grutil;

// Thread safety across instances
std::mutex genalyzer_fft_vii_impl::s_genalyzer_mutex;

genalyzer_fft_vii::sptr genalyzer_fft_vii::make(int npts, int qres, int navg, int nfft, GnWindow win,
						double sample_rate, bool do_shift)
{
	return std::make_shared<genalyzer_fft_vii_impl>(npts, qres, navg, nfft, win, sample_rate, do_shift);
}

genalyzer_fft_vii_impl::genalyzer_fft_vii_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate,
					       bool do_shift)
	: genalyzer_fft_vii("genalyzer_fft_vii", gr::io_signature::make(2, 2, sizeof(int32_t) * nfft),
			    gr::io_signature::make(1, 1, sizeof(float) * nfft))
	, d_npts(npts)
	, d_qres(qres)
	, d_navg(navg)
	, d_nfft(nfft)
	, d_win(win)
	, d_sample_rate(sample_rate)
	, d_do_shift(do_shift)
	, d_genalyzer_config() // Initialize with default config
	, d_fa_key(nullptr)
	, d_fft_out(nullptr)
	, d_qwfi(nullptr)
	, d_qwfq(nullptr)
	, d_frame_buffers_i(nullptr)
	, d_frame_buffers_q(nullptr)
	, d_current_frame_index(0)
	, d_frames_filled(0)
	, d_allocated_frames(0)
	, d_config(nullptr)
	, d_analysis(new gn_analysis_results)
{
	allocate_buffers();
}

genalyzer_fft_vii_impl::~genalyzer_fft_vii_impl() { cleanup_buffers(); }

void genalyzer_fft_vii_impl::allocate_buffers()
{
	d_fft_out = nullptr;

	d_qwfi = new int32_t[d_npts]();
	d_qwfq = new int32_t[d_npts]();

	d_frame_buffers_i = new int32_t *[d_navg];
	d_frame_buffers_q = new int32_t *[d_navg];

	for(int i = 0; i < d_navg; i++) {
		d_frame_buffers_i[i] = new int32_t[d_nfft]();
		d_frame_buffers_q[i] = new int32_t[d_nfft]();
	}

	d_current_frame_index = 0;
	d_frames_filled = 0;
	d_allocated_frames = d_navg;
}

void genalyzer_fft_vii_impl::cleanup_frame_buffers()
{
	if(d_frame_buffers_i) {
		for(size_t i = 0; i < d_allocated_frames; i++) {
			delete[] d_frame_buffers_i[i];
		}
		delete[] d_frame_buffers_i;
		d_frame_buffers_i = nullptr;
	}
	if(d_frame_buffers_q) {
		for(size_t i = 0; i < d_allocated_frames; i++) {
			delete[] d_frame_buffers_q[i];
		}
		delete[] d_frame_buffers_q;
		d_frame_buffers_q = nullptr;
	}
	d_allocated_frames = 0;
	d_current_frame_index = 0;
	d_frames_filled = 0;
}

void genalyzer_fft_vii_impl::cleanup_buffers()
{
	if(d_fft_out) {
		free(d_fft_out);
		d_fft_out = nullptr;
	}
	if(d_qwfi) {
		delete[] d_qwfi;
		d_qwfi = nullptr;
	}
	if(d_qwfq) {
		delete[] d_qwfq;
		d_qwfq = nullptr;
	}

	if(d_analysis) {
		delete d_analysis;
		d_analysis = nullptr;
	}

	cleanup_frame_buffers();
	cleanup_fa_config();
	cleanup_auto_config();
}

int genalyzer_fft_vii_impl::configure_auto_analysis()
{
	cleanup_auto_config();

	int err_code = gn_config_fftz(d_npts, d_qres, d_navg, d_nfft, d_win, &d_config);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_config_fftz failed with error code: " + std::to_string(err_code));
		return err_code;
	}

	err_code = gn_config_set_sample_rate(d_sample_rate, &d_config);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_config_set_sample_rate failed with error code: " + std::to_string(err_code));
		return err_code;
	}

	err_code = gn_config_fa_auto(d_genalyzer_config.auto_params.ssb_width, &d_config);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "Failed to run gn_config_fa_auto. Error code: " + std::to_string(err_code));
		return err_code;
	}

	return 0;
}

bool genalyzer_fft_vii_impl::start() { return true; }

bool genalyzer_fft_vii_impl::stop() { return true; }

void genalyzer_fft_vii_impl::set_sample_rate(double sample_rate) { d_sample_rate = sample_rate; }

double genalyzer_fft_vii_impl::sample_rate() const { return d_sample_rate; }

void genalyzer_fft_vii_impl::set_window(GnWindow win) { d_win = win; }

int genalyzer_fft_vii_impl::window() const { return d_win; }

int genalyzer_fft_vii_impl::navg() const { return d_navg; }

void genalyzer_fft_vii_impl::set_ssb_width(uint8_t ssb_width)
{
	d_genalyzer_config.auto_params.ssb_width = ssb_width;

	cleanup_auto_config();
	configure_auto_analysis();
}

uint8_t genalyzer_fft_vii_impl::ssb_width() const { return d_genalyzer_config.auto_params.ssb_width; }

void genalyzer_fft_vii_impl::set_config(const GenalyzerConfig &config)
{
	std::lock_guard<std::mutex> lock(s_genalyzer_mutex);

	cleanup_fa_config();
	cleanup_auto_config();

	d_genalyzer_config = config;
}

GenalyzerConfig genalyzer_fft_vii_impl::get_config() const { return d_genalyzer_config; }

void genalyzer_fft_vii_impl::clear_analysis_results()
{
	d_analysis->results_size = 0;
	d_analysis->rkeys = nullptr;
	d_analysis->rvalues = nullptr;
}

void genalyzer_fft_vii_impl::set_analysis_results(size_t results_size, char **rkeys, double *rvalues)
{
	d_analysis->results_size = results_size;
	d_analysis->rkeys = rkeys;
	d_analysis->rvalues = rvalues;
}

void genalyzer_fft_vii_impl::free_analysis_keys(char **rkeys, size_t count)
{
	if(rkeys) {
		for(size_t i = 0; i < count; ++i) {
			if(rkeys[i]) {
				free(rkeys[i]);
			}
		}
		free(rkeys);
	}
}

void genalyzer_fft_vii_impl::store_frame_to_circular_buffer(const int32_t *in_i_vec, const int32_t *in_q_vec)
{
	for(size_t j = 0; j < d_nfft; j++) {
		d_frame_buffers_i[d_current_frame_index][j] = in_i_vec[j];
		d_frame_buffers_q[d_current_frame_index][j] = in_q_vec[j];
	}

	d_current_frame_index = (d_current_frame_index + 1) % d_navg;
	if(d_frames_filled < d_navg) {
		d_frames_filled++;
	}
}

size_t genalyzer_fft_vii_impl::prepare_frames_for_processing(size_t &current_npts)
{
	size_t frames_to_process = std::min(d_frames_filled, d_navg);
	if(frames_to_process == 0) {
		return 0;
	}

	// Copy frames chronologically from oldest
	size_t oldest_frame_index = (d_frames_filled < d_navg) ? 0 : d_current_frame_index;

	for(size_t frame = 0; frame < frames_to_process; frame++) {
		size_t source_frame_index = (oldest_frame_index + frame) % d_navg;
		size_t dest_offset = frame * d_nfft;

		for(size_t j = 0; j < d_nfft; j++) {
			d_qwfi[dest_offset + j] = d_frame_buffers_i[source_frame_index][j];
			d_qwfq[dest_offset + j] = d_frame_buffers_q[source_frame_index][j];
		}
	}
	current_npts = frames_to_process * d_nfft;
	return frames_to_process;
}

int genalyzer_fft_vii_impl::perform_fft_and_convert_to_db(size_t frames_to_process, size_t current_npts,
							  double *shifted_output, double *db_output, float *out_vec)
{
	if(!d_qwfi || !d_qwfq) {
		GR_LOG_ERROR(d_logger, "Input quantization buffers are null");
		return -1;
	}

	free(d_fft_out);
	d_fft_out = (double *)calloc(2 * d_nfft, sizeof(double));

	if(gn_fft32(d_fft_out, 2 * d_nfft, d_qwfi, current_npts, d_qwfq, current_npts, d_qres, frames_to_process,
		    d_nfft, d_win, GnCodeFormatTwosComplement) != 0) {
		GR_LOG_ERROR(d_logger, "gn_fft32 failed");
		return -1;
	}

	// Apply FFT shift for complex mode or convert directly to dB for float mode
	if(d_do_shift) {
		if(gn_ifftshift(shifted_output, 2 * d_nfft, d_fft_out, 2 * d_nfft) != 0) {
			GR_LOG_ERROR(d_logger, "gn_ifftshift failed");
			return -1;
		}
		if(gn_db(db_output, d_nfft, shifted_output, 2 * d_nfft) != 0) {
			GR_LOG_ERROR(d_logger, "gn_db failed");
			return -1;
		}
	} else {
		if(gn_db(db_output, d_nfft, d_fft_out, 2 * d_nfft) != 0) {
			GR_LOG_ERROR(d_logger, "gn_db failed");
			return -1;
		}
	}

	// Convert to float output
	for(size_t j = 0; j < d_nfft; j++) {
		out_vec[j] = static_cast<float>(db_output[j]);
	}

	return 0;
}

void genalyzer_fft_vii_impl::cleanup_fa_config()
{
	if(d_fa_key) {
		gn_fa_reset(d_fa_key);
		free(d_fa_key);
		d_fa_key = nullptr;
	}
}

void genalyzer_fft_vii_impl::cleanup_auto_config()
{
	if(d_config) {
		gn_config_free(&d_config);
		d_config = nullptr;
	}
}

int genalyzer_fft_vii_impl::configure_fixed_tone_analysis()
{

	// Clean up any existing configuration
	cleanup_fa_config();

	// Create unique key for this configuration
	d_fa_key = strdup("fa");

	int err_code = 0;

	// Create FA configuration
	err_code = gn_fa_create(d_fa_key);
	const auto &ft = d_genalyzer_config.fixed_tone;

	// Define fixed tone (fundamental)
	err_code = gn_fa_fixed_tone(d_fa_key, ft.component_label.c_str(), GnFACompTagSignal, ft.expected_freq,
				    ft.ssb_fundamental);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_fixed_tone failed: " + std::to_string(err_code));
		return err_code;
	}

	// Configure harmonic distortion analysis if enabled
	if(ft.harmonic_order > 0) {
		err_code = gn_fa_hd(d_fa_key, ft.harmonic_order);
		if(err_code != 0) {
			GR_LOG_ERROR(d_logger, "gn_fa_hd failed: " + std::to_string(err_code));
			return err_code;
		}
	}

	// Configure SSB groups
	err_code = gn_fa_ssb(d_fa_key, GnFASsbDefault, ft.ssb_default);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_ssb Default failed: " + std::to_string(err_code));
		return err_code;
	}
	err_code = gn_fa_ssb(d_fa_key, GnFASsbDC, -1);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_ssb DC failed: " + std::to_string(err_code));
		return err_code;
	}
	err_code = gn_fa_ssb(d_fa_key, GnFASsbSignal, -1);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_ssb Signal failed: " + std::to_string(err_code));
		return err_code;
	}
	err_code = gn_fa_ssb(d_fa_key, GnFASsbWO, -1);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_ssb WO failed: " + std::to_string(err_code));
		return err_code;
	}

	// Set frequency parameters
	err_code = gn_fa_fsample(d_fa_key, d_sample_rate);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_fsample failed: " + std::to_string(err_code));
		return err_code;
	}

	err_code = gn_fa_fdata(d_fa_key, d_sample_rate); // Assuming no decimation
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_fdata failed: " + std::to_string(err_code));
		return err_code;
	}

	err_code = gn_fa_fshift(d_fa_key, ft.fshift);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_fshift failed: " + std::to_string(err_code));
		return err_code;
	}

	err_code = gn_fa_conv_offset(d_fa_key, 0.0 != ft.fshift);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fa_conv_offset failed: " + std::to_string(err_code));
		return err_code;
	}

	return 0;
}

void genalyzer_fft_vii_impl::perform_fixed_tone_analysis()
{
	// Configure fixed tone analysis only if not already configured
	if(!d_fa_key) {
		int config_result = configure_fixed_tone_analysis();
		if(config_result != 0) {
			GR_LOG_ERROR(d_logger, "Failed to configure fixed tone analysis");
			clear_analysis_results();
			cleanup_fa_config();
			return;
		}
	}

	size_t results_size = 0;

	// Get results size first
	int err_code = gn_fft_analysis_results_size(&results_size, d_fa_key, 2 * d_nfft, d_nfft);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fft_analysis_results_size failed: " + std::to_string(err_code));
		clear_analysis_results();
		return;
	}

	// Get key sizes
	size_t *rkey_sizes = (size_t *)malloc(results_size * sizeof(size_t));
	err_code = gn_fft_analysis_results_key_sizes(rkey_sizes, results_size, d_fa_key, 2 * d_nfft, d_nfft);
	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fft_analysis_results_key_sizes failed: " + std::to_string(err_code));
		free(rkey_sizes);
		clear_analysis_results();
		return;
	}

	// Allocate memory for results
	char **rkeys = (char **)malloc(results_size * sizeof(char *));
	double *rvalues = (double *)malloc(results_size * sizeof(double));

	// Allocate memory for each key
	for(size_t i = 0; i < results_size; ++i) {
		rkeys[i] = (char *)malloc(rkey_sizes[i]);
	}
	free(rkey_sizes);

	// Execute analysis
	err_code = gn_fft_analysis(rkeys, results_size, rvalues, results_size, d_fa_key, d_fft_out, 2 * d_nfft, d_nfft,
				   GnFreqAxisTypeDcLeft); // data is unshifter so we use left DC

	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "gn_fft_analysis failed: " + std::to_string(err_code));
		free(rkeys);
		free(rvalues);
		clear_analysis_results();
	} else {
		set_analysis_results(results_size, rkeys, rvalues);
	}
}

void genalyzer_fft_vii_impl::perform_auto_analysis()
{
	size_t results_size = 0;
	char **rkeys = nullptr;
	double *rvalues = nullptr;

	// Configure genalyzer for auto mode only if not already configured
	if(!d_config) {
		int config_err = configure_auto_analysis();
		if(config_err != 0) {
			GR_LOG_ERROR(d_logger,
				     "Failed to configure genalyzer. Error code: " + std::to_string(config_err));
			clear_analysis_results();
			cleanup_auto_config();
			return;
		}
	}

	int err_code = gn_get_fa_results(&rkeys, &rvalues, &results_size, d_fft_out, &d_config);
	if(err_code == 0) {
		set_analysis_results(results_size, rkeys, rvalues);
	} else {
		GR_LOG_ERROR(d_logger, "Failed to compute Genalyzer analysis. Error code: " + std::to_string(err_code));
		clear_analysis_results();
	}
}

void genalyzer_fft_vii_impl::perform_genalyzer_analysis()
{
	if(d_genalyzer_config.isFixedToneMode()) {
		perform_fixed_tone_analysis();
	} else {
		perform_auto_analysis();
	}
}

int genalyzer_fft_vii_impl::work(int noutput_items, gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	{
		std::lock_guard<std::mutex> lock(s_genalyzer_mutex);

		const int32_t *in_i = (const int32_t *)input_items[0];
		const int32_t *in_q = (const int32_t *)input_items[1];
		float *out = (float *)output_items[0];

		double *shifted_output = (double *)calloc(2 * d_nfft, sizeof(double));
		double *db_output = (double *)calloc(d_nfft, sizeof(double));

		for(int i = 0; i < noutput_items; i++) {
			const int32_t *in_i_vec = in_i + (i * d_nfft);
			const int32_t *in_q_vec = in_q + (i * d_nfft);
			float *out_vec = out + (i * d_nfft);

			if(!d_frame_buffers_i || !d_frame_buffers_q || d_navg == 0) {
				std::memset(out_vec, 0, d_nfft * sizeof(float));
				continue;
			}

			// Store new frame to circular buffer
			store_frame_to_circular_buffer(in_i_vec, in_q_vec);

			// Prepare frames for FFT processing
			size_t current_npts;
			size_t frames_to_process = prepare_frames_for_processing(current_npts);
			if(frames_to_process == 0) {
				std::memset(out_vec, 0, d_nfft * sizeof(float));
				continue;
			}

			// Perform FFT and convert to dB
			if(perform_fft_and_convert_to_db(frames_to_process, current_npts, shifted_output, db_output,
							 out_vec) < 0) {
				free(shifted_output);
				free(db_output);
				return -1;
			}
		}

		if(d_genalyzer_config.enabled) {
			// unshifted raw fft data from gn_fft32() is used in analysis (d_fft_out)
			perform_genalyzer_analysis();
		}
		free(shifted_output);
		free(db_output);
	}

	return noutput_items;
}

gn_analysis_results *genalyzer_fft_vii_impl::getGnAnalysis() { return d_analysis; }
