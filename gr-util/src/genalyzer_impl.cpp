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

genalyzer_fft_vii_impl::genalyzer_fft_vii_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate, bool do_shift)
	: genalyzer_fft_vii("genalyzer_fft_vii", gr::io_signature::make(2, 2, sizeof(int32_t) * nfft),
			    gr::io_signature::make(1, 1, sizeof(float) * nfft))
	, d_npts(npts)
	, d_qres(qres)
	, d_navg(navg)
	, d_nfft(nfft)
	, d_win(win)
	, d_sample_rate(sample_rate)
	, d_do_shift(do_shift)
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

	cleanup_frame_buffers();

	if(d_analysis) {
		delete d_analysis;
		d_analysis = nullptr;
	}
	if(d_config) {
		gn_config_free(&d_config);
		d_config = nullptr;
	}
}

int genalyzer_fft_vii_impl::configure_genalyzer()
{
	if(d_config) {
		gn_config_free(&d_config);
		d_config = nullptr;
	}

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

	return 0;
}

bool genalyzer_fft_vii_impl::start() { return true; }

bool genalyzer_fft_vii_impl::stop() { return true; }

void genalyzer_fft_vii_impl::set_sample_rate(double sample_rate) { d_sample_rate = sample_rate; }

double genalyzer_fft_vii_impl::sample_rate() const { return d_sample_rate; }

void genalyzer_fft_vii_impl::set_window(GnWindow win) { d_win = win; }

int genalyzer_fft_vii_impl::window() const { return d_win; }

void genalyzer_fft_vii_impl::set_navg(int navg)
{
	if(navg <= 0) {
		GR_LOG_ERROR(d_logger, "Invalid navg value: " + std::to_string(navg));
		return;
	}

	std::lock_guard<std::mutex> lock(d_buffer_mutex);
	d_navg = navg;
	d_npts = navg * d_nfft;

	if(d_qwfi) {
		delete[] d_qwfi;
		d_qwfi = nullptr;
	}
	if(d_qwfq) {
		delete[] d_qwfq;
		d_qwfq = nullptr;
	}
	cleanup_frame_buffers();

	d_qwfi = new int32_t[d_npts]();
	d_qwfq = new int32_t[d_npts]();

	d_frame_buffers_i = new int32_t *[navg];
	d_frame_buffers_q = new int32_t *[navg];

	for(int i = 0; i < navg; i++) {
		d_frame_buffers_i[i] = new int32_t[d_nfft]();
		d_frame_buffers_q[i] = new int32_t[d_nfft]();
	}

	d_current_frame_index = 0;
	d_frames_filled = 0;
	d_allocated_frames = navg;
}

int genalyzer_fft_vii_impl::navg() const { return d_navg; }

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

			size_t frames_to_process, current_npts;

			if(!d_frame_buffers_i || !d_frame_buffers_q || d_navg == 0) {
				std::memset(out_vec, 0, d_nfft * sizeof(float));
				continue;
			}

			for(size_t j = 0; j < d_nfft; j++) {
				d_frame_buffers_i[d_current_frame_index][j] = in_i_vec[j];
				d_frame_buffers_q[d_current_frame_index][j] = in_q_vec[j];
			}

			d_current_frame_index = (d_current_frame_index + 1) % d_navg;
			if(d_frames_filled < d_navg) {
				d_frames_filled++;
			}

			frames_to_process = std::min(d_frames_filled, d_navg);
			if(frames_to_process == 0) {
				std::memset(out_vec, 0, d_nfft * sizeof(float));
				continue;
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

			if(!d_qwfi || !d_qwfq) {
				GR_LOG_ERROR(d_logger, "Input quantization buffers are null");
				return -1;
			}

			free(d_fft_out);
			d_fft_out = (double *)calloc(2 * d_nfft, sizeof(double));

			if(gn_fft32(d_fft_out, 2 * d_nfft, d_qwfi, current_npts, d_qwfq, current_npts, d_qres,
				    frames_to_process, d_nfft, d_win, GnCodeFormatTwosComplement) != 0) {
				GR_LOG_ERROR(d_logger, "gn_fft32 failed");
				free(shifted_output);
				free(db_output);
				return -1;
			}

			if(d_do_shift) {
				if(gn_ifftshift(shifted_output, 2 * d_nfft, d_fft_out, 2 * d_nfft) != 0) {
					GR_LOG_ERROR(d_logger, "gn_ifftshift failed");
					free(shifted_output);
					free(db_output);
					return -1;
				}
				if(gn_db(db_output, d_nfft, shifted_output, 2 * d_nfft) != 0) {
					GR_LOG_ERROR(d_logger, "gn_db failed");
					free(shifted_output);
					free(db_output);
					return -1;
				}
			} else {
				// For float mode, no shift - directly convert to dB
				if(gn_db(db_output, d_nfft, d_fft_out, 2 * d_nfft) != 0) {
					GR_LOG_ERROR(d_logger, "gn_db failed");
					free(shifted_output);
					free(db_output);
					return -1;
				}
			}

			for(size_t j = 0; j < d_nfft; j++) {
				out_vec[j] = static_cast<float>(db_output[j]);
			}
		}

		// Only perform analysis for complex mode
		if(d_do_shift) {
			size_t results_size = 0;
			char **rkeys = nullptr;
			double *rvalues = nullptr;

			configure_genalyzer();
			uint8_t ssb_width = 120;
			int err_code = gn_config_fa_auto(ssb_width, &d_config);
			if(err_code == 0) {
				err_code = gn_get_fa_results(&rkeys, &rvalues, &results_size, d_fft_out, &d_config);
			} else {
				GR_LOG_ERROR(d_logger,
					     "Failed to run gn_config_fa_auto. Error code: " + std::to_string(err_code));
			}

			if(err_code != 0) {
				GR_LOG_ERROR(d_logger,
					     "Failed to compute Genalyzer analysis. Error code: " + std::to_string(err_code));

				d_analysis->results_size = 0;
				d_analysis->rkeys = nullptr;
				d_analysis->rvalues = nullptr;
			} else {
				d_analysis->results_size = results_size;
				d_analysis->rkeys = rkeys;
				d_analysis->rvalues = rvalues;
			}
		} else {
			d_analysis->results_size = 0;
			d_analysis->rkeys = nullptr;
			d_analysis->rvalues = nullptr;
		}

		free(shifted_output);
		free(db_output);
	}

	return noutput_items;
}

gn_analysis_results *genalyzer_fft_vii_impl::getGnAnalysis() { return d_analysis; }
