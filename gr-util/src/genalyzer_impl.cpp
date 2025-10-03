#include "genalyzer.h"
#include "genalyzer_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <cgenalyzer_simplified_beta.h>
#include <cgenalyzer.h>

using namespace scopy::grutil;

// Define static mutex for thread safety across all instances
std::mutex genalyzer_fft_vii_impl::s_genalyzer_mutex;

genalyzer_fft_vii::sptr genalyzer_fft_vii::make(int npts, int qres, int navg, int nfft, GnWindow win,
						double sample_rate)
{
	return std::make_shared<genalyzer_fft_vii_impl>(npts, qres, navg, nfft, win, sample_rate);
}

genalyzer_fft_vii_impl::genalyzer_fft_vii_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate)
	: genalyzer_fft_vii("genalyzer_fft_vii", gr::io_signature::make(2, 2, sizeof(int32_t) * npts),
			    gr::io_signature::make(1, 1, sizeof(float) * nfft))
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
	, d_analysis_enabled(false) // Default to disabled to avoid unnecessary computation
{
	allocate_buffers();
}

genalyzer_fft_vii_impl::~genalyzer_fft_vii_impl() { cleanup_buffers(); }

void genalyzer_fft_vii_impl::allocate_buffers()
{
	d_fft_out = nullptr;

	d_qwfi = new int32_t[d_npts];
	d_qwfq = new int32_t[d_npts];

	std::memset(d_qwfi, 0, sizeof(int32_t) * d_npts);
	std::memset(d_qwfq, 0, sizeof(int32_t) * d_npts);
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

int genalyzer_fft_vii_impl::work(int noutput_items, gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items)
{
	{
		std::lock_guard<std::mutex> lock(s_genalyzer_mutex);

		configure_genalyzer();

		// Input 0: I channel (int32_t), Input 1: Q channel (int32_t)
		const int32_t *in_i = (const int32_t *)input_items[0];
		const int32_t *in_q = (const int32_t *)input_items[1];
		float *out = (float *)output_items[0];

		// Allocate buffers for shifted output and dB conversion
		double *shifted_output = (double *)calloc(2 * d_nfft, sizeof(double));
		double *db_output = (double *)calloc(d_nfft, sizeof(double));

		for(int i = 0; i < noutput_items; i++) {
			// Get pointers to current input vectors
			const int32_t *in_i_vec = in_i + (i * d_npts);
			const int32_t *in_q_vec = in_q + (i * d_npts);

			// Copy int32_t data directly - no conversion needed!
			for(size_t j = 0; j < d_npts; j++) {
				d_qwfi[j] = in_i_vec[j];
				d_qwfq[j] = in_q_vec[j];
			}

			// Validate config before calling gn_fftz
			if(!d_config) {
				GR_LOG_ERROR(d_logger, "genalyzer config is null - reinitializing");
				if(configure_genalyzer() != 0) {
					GR_LOG_ERROR(d_logger, "Failed to reinitialize genalyzer config");
					return -1;
				}
			}

			// Validate input buffers
			if(!d_qwfi || !d_qwfq) {
				GR_LOG_ERROR(d_logger, "Input quantization buffers are null");
				return -1;
			}

			if(d_fft_out) {
				free(d_fft_out);
				d_fft_out = nullptr;
			}
			int err_code = gn_fftz(&d_fft_out, d_qwfi, d_qwfq, &d_config);
			if(err_code != 0) {
				GR_LOG_ERROR(d_logger, "gn_fftz failed with error code: " + std::to_string(err_code));
				return -1; // Signal error to GNU Radio
			}

			// Critical: Check if gn_fftz actually allocated the output buffer
			if(!d_fft_out) {
				GR_LOG_ERROR(d_logger, "gn_fftz failed to allocate output buffer");
				return -1;
			}

			// Get pointer to current output vector
			float *out_vec = out + (i * d_nfft);

			// Use genalyzer's ifftshift function for proper FFT shifting
			int err_code_shift = gn_ifftshift(shifted_output, 2 * d_nfft, d_fft_out, 2 * d_nfft);

			if(err_code_shift != 0) {
				GR_LOG_ERROR(d_logger, "gn_ifftshift failed with error code: " + std::to_string(err_code_shift));
				free(shifted_output);
				free(db_output);
				return -1;
			}

			// Apply gn_db() to convert complex data to magnitude in dB
			int err_code_db = gn_db(db_output, d_nfft, shifted_output, 2 * d_nfft);

			if(err_code_db != 0) {
				GR_LOG_ERROR(d_logger, "gn_db failed with error code: " + std::to_string(err_code_db));
				free(shifted_output);
				free(db_output);
				return -1;
			}

			// Convert dB magnitude values to float
			for(size_t j = 0; j < d_nfft; j++) {
				out_vec[j] = static_cast<float>(db_output[j]);
			}
		}

		// Only compute Fourier analysis if explicitly enabled (use shifted_output for analysis)
		if(true) {
			size_t results_size = 0;
			char **rkeys = nullptr;
			double *rvalues = nullptr;

			// Use appropriate SSB width based on window type (not quantization resolution!)
			uint8_t ssb_width = (d_win == GnWindowNoWindow) ? 0 : 4;
			int err_code = gn_config_fa_auto(ssb_width, &d_config);
			if(err_code == 0) {
				err_code = gn_get_fa_results(&rkeys, &rvalues, &results_size, shifted_output, &d_config);
			} else {
				GR_LOG_ERROR(d_logger,
					     "Failed to run gn_config_fa_auto. Error code: " +
						     std::to_string(err_code));
			}

			if(err_code != 0) {
				GR_LOG_ERROR(d_logger,
					     "Failed to compute Genalyzer analysis. Error code: " +
						     std::to_string(err_code));

				// Reset analysis results on error to prevent stale data
				d_analysis->results_size = 0;
				d_analysis->rkeys = nullptr;
				d_analysis->rvalues = nullptr;
			} else {
				d_analysis->results_size = results_size;
				d_analysis->rkeys = rkeys;
				d_analysis->rvalues = rvalues;
			}
		}

		// Clean up allocated buffers
		free(shifted_output);
		free(db_output);
	}

	return noutput_items;
}

gn_analysis_results *genalyzer_fft_vii_impl::getGnAnalysis() { return d_analysis; }

void genalyzer_fft_vii_impl::setAnalysisEnabled(bool enabled) { d_analysis_enabled = enabled; }

bool genalyzer_fft_vii_impl::analysisEnabled() const { return d_analysis_enabled; }
