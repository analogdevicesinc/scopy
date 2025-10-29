#ifndef INCLUDED_GENALYZER_FFT_VCC_H
#define INCLUDED_GENALYZER_FFT_VCC_H

#include "scopy-gr-util_export.h"
#include <gnuradio/sync_block.h>
#include <cgenalyzer_simplified_beta.h>

namespace scopy::grutil {

struct gn_analysis_results
{
	size_t results_size = 0;
	char **rkeys = nullptr;
	double *rvalues = nullptr;
};

class SCOPY_GR_UTIL_EXPORT genalyzer_fft_vii : public gr::sync_block
{
public:
	typedef std::shared_ptr<genalyzer_fft_vii> sptr;

	/*!
	 * \brief Create a new instance of genalyzer_fft_vii (int32 input, complex output)
	 * \param npts Number of points in FFT
	 * \param qres Quantizer resolution (bits) for genalyzer internal processing
	 * \param navg Number of averages
	 * \param nfft FFT size
	 * \param win Window type (GN_WINDOW_*)
	 * \param sample_rate Sample rate in Hz
	 */
	static sptr make(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate);

	virtual void set_sample_rate(double sample_rate) = 0;
	virtual double sample_rate() const = 0;
	virtual void set_window(GnWindow win) = 0;
	virtual int window() const = 0;
	virtual void set_navg(int navg) = 0;
	virtual int navg() const = 0;
	virtual gn_analysis_results *getGnAnalysis() = 0;
	virtual void setAnalysisEnabled(bool enabled) = 0;
	virtual bool analysisEnabled() const = 0;

protected:
	genalyzer_fft_vii(const std::string &name, gr::io_signature::sptr input_signature,
			  gr::io_signature::sptr output_signature)
		: gr::sync_block(name, input_signature, output_signature)
	{}
};

} // namespace scopy::grutil

#endif /* INCLUDED_GENALYZER_FFT_VCC_H */
