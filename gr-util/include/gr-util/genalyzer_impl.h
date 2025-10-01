#ifndef INCLUDED_GENALYZER_FFT_VCC_IMPL_H
#define INCLUDED_GENALYZER_FFT_VCC_IMPL_H

#include "genalyzer.h"
#include <cgenalyzer_simplified_beta.h>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT genalyzer_fft_vcc_impl : public genalyzer_fft_vcc
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
	int32_t *d_qwfi; // Quantized waveform I
	int32_t *d_qwfq; // Quantized waveform Q

	gn_analysis_results *d_analysis;

	void cleanup_buffers();
	void allocate_buffers();
	int configure_genalyzer();

public:
	genalyzer_fft_vcc_impl(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate);
	~genalyzer_fft_vcc_impl();

	void set_sample_rate(double sample_rate) override;
	double sample_rate() const override;
	void set_window(GnWindow win) override;
	int window() const override;

	bool start() override;
	bool stop() override;
	int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) override;

	gn_analysis_results *getGnAnalysis() override;
};

} // namespace scopy::grutil

#endif /* INCLUDED_GENALYZER_FFT_VCC_IMPL_H */
