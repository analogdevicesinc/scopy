#include "genalyzer.h"
#include "genalyzer_impl.h"
#include <gnuradio/io_signature.h>
#include <stdexcept>
#include <cstring>
#include <cgenalyzer_simplified_beta.h>

using namespace scopy::grutil;

genalyzer_fft_vcc::sptr genalyzer_fft_vcc::make(int npts, int qres, int navg, int nfft, GnWindow win,
						double sample_rate)
{
	return std::make_shared<genalyzer_fft_vcc_impl>(npts, qres, navg, nfft, win, sample_rate);
}

genalyzer_fft_vcc_impl::genalyzer_fft_vcc_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate)
	: genalyzer_fft_vcc("genalyzer_fft_vcc", gr::io_signature::make(1, 1, sizeof(gr_complex) * npts),
			    gr::io_signature::make(1, 1, sizeof(gr_complex) * nfft))
	, d_npts(npts)
	, d_qres(qres)
	, d_navg(navg)
	, d_nfft(nfft)
	, d_win(win)
	, d_sample_rate(sample_rate)
	, d_fft_out(nullptr)
	, d_qwfi(nullptr)
	, d_qwfq(nullptr)
	, d_config(nullptr)
	, d_analysis(new gn_analysis_results)
	, d_previous_rkeys(nullptr)
	, d_previous_rvalues(nullptr)
{
	allocate_buffers();
	configure_genalyzer();
}

genalyzer_fft_vcc_impl::~genalyzer_fft_vcc_impl() { cleanup_buffers(); }

void genalyzer_fft_vcc_impl::allocate_buffers()
{
	// Don't allocate d_fft_out here - genalyzer will allocate it
	d_fft_out = nullptr;

	d_qwfi = new int32_t[d_npts];
	d_qwfq = new int32_t[d_npts];

	std::memset(d_qwfi, 0, sizeof(int32_t) * d_npts);
	std::memset(d_qwfq, 0, sizeof(int32_t) * d_npts);
}

void genalyzer_fft_vcc_impl::cleanup_buffers()
{
	if(d_fft_out) {
		// Use genalyzer's memory free function if it exists
		// Otherwise use standard free() since genalyzer likely uses malloc
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

	// Free previous analysis results memory
	cleanup_analysis_results();

	if(d_analysis) {
		delete d_analysis;
		d_analysis = nullptr;
	}

	if(d_config) {
		gn_config_free(&d_config);
		d_config = nullptr;
	}
}

void genalyzer_fft_vcc_impl::cleanup_analysis_results()
{
	// Free previously allocated analysis results to prevent memory leaks
	if(d_previous_rkeys) {
		// Free each string in the rkeys array
		for(size_t i = 0; i < d_analysis->results_size; i++) {
			if(d_previous_rkeys[i]) {
				free(d_previous_rkeys[i]);
			}
		}
		free(d_previous_rkeys);
		d_previous_rkeys = nullptr;
	}

	if(d_previous_rvalues) {
		free(d_previous_rvalues);
		d_previous_rvalues = nullptr;
	}
}

int genalyzer_fft_vcc_impl::configure_genalyzer()
{
	int err_code = 0;

	err_code = gn_config_fftz(d_npts, d_qres, d_navg, d_nfft, d_win, &d_config);
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

bool genalyzer_fft_vcc_impl::start()
{
	return true;
}

bool genalyzer_fft_vcc_impl::stop() { return true; }

void genalyzer_fft_vcc_impl::set_sample_rate(double sample_rate)
{
	d_sample_rate = sample_rate;
	if(d_config) {
		int err_code = gn_config_set_sample_rate(d_sample_rate, &d_config);
		if(err_code != 0) {
			GR_LOG_ERROR(d_logger, "Failed to update sample rate");
		}
	}
}

double genalyzer_fft_vcc_impl::sample_rate() const { return d_sample_rate; }

void genalyzer_fft_vcc_impl::set_window(GnWindow win)
{
	d_win = win;
	// Reconfigure if needed
	if(d_config) {
		configure_genalyzer();
	}
}

int genalyzer_fft_vcc_impl::window() const { return d_win; }

int genalyzer_fft_vcc_impl::work(int noutput_items, gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex *)input_items[0];
	gr_complex *out = (gr_complex *)output_items[0];

	for(int i = 0; i < noutput_items; i++) {
		// Get pointer to current input vector
		const gr_complex *in_vec = in + (i * d_npts);

		// Convert gr_complex input to separate I/Q int32_t arrays
		// CRITICAL: Need to scale the float values to int32 range based on qres
		// Assuming input is normalized between -1.0 and 1.0
		double scale = (1 << (d_qres)); // 2^(qres-1) for signed integers

		for(size_t j = 0; j < d_npts; j++) {
			// Scale and quantize the input samples
			d_qwfi[j] = static_cast<int32_t>(in_vec[j].real() * scale);
			d_qwfq[j] = static_cast<int32_t>(in_vec[j].imag() * scale);
		}

		// Perform genalyzer FFT
		if(d_fft_out) {
			free(d_fft_out);
			d_fft_out = nullptr;
		}
		int err_code = gn_fftz(&d_fft_out, d_qwfi, d_qwfq, &d_config);
		if(err_code != 0) {
			GR_LOG_ERROR(d_logger, "gn_fftz failed with error code: " + std::to_string(err_code));
			return -1; // Signal error to GNU Radio
		}

		// Get pointer to current output vector
		gr_complex *out_vec = out + (i * d_nfft);

		// Convert interleaved Re/Im FFT output back to gr_complex
		// AND perform FFT shift: move second half to beginning, first half to end
		// This puts negative frequencies on left, DC in center, positive frequencies on right
		size_t half = d_nfft / 2;

		// Copy second half of FFT output to first half of output
		for(size_t j = 0; j < half; j++) {
			size_t src_idx = (half + j) * 2; // Start from middle of FFT output
			double real_part = d_fft_out[src_idx];
			double imag_part = d_fft_out[src_idx + 1];
			out_vec[j] = gr_complex(static_cast<float>(real_part), static_cast<float>(imag_part));
		}

		// Copy first half of FFT output to second half of output
		for(size_t j = 0; j < half; j++) {
			size_t src_idx = j * 2; // Start from beginning of FFT output
			double real_part = d_fft_out[src_idx];
			double imag_part = d_fft_out[src_idx + 1];
			out_vec[half + j] = gr_complex(static_cast<float>(real_part), static_cast<float>(imag_part));
		}

		// Handle odd FFT size (middle element stays in place)
		if(d_nfft % 2 != 0) {
			size_t mid_idx = half * 2;
			out_vec[half] = gr_complex(static_cast<float>(d_fft_out[mid_idx]),
						   static_cast<float>(d_fft_out[mid_idx + 1]));
		}
	}

	// Configure Fourier analysis
	size_t results_size;
	char **rkeys;
	double *rvalues;

	// Free previous analysis results before allocating new ones
	cleanup_analysis_results();

	int err_code = gn_config_fa_auto(d_qres - 1, &d_config);
	err_code = gn_get_fa_results(&rkeys, &rvalues, &results_size, d_fft_out, &d_config);

	if(err_code != 0) {
		GR_LOG_ERROR(d_logger, "Failed to compute Genalyzer analysis.");
	} else {
		// Store references to current results for cleanup later
		d_previous_rkeys = rkeys;
		d_previous_rvalues = rvalues;

		d_analysis->results_size = results_size;
		d_analysis->rkeys = rkeys;
		d_analysis->rvalues = rvalues;
	}

	return noutput_items;
}

gn_analysis_results *genalyzer_fft_vcc_impl::getGnAnalysis() { return d_analysis; }
