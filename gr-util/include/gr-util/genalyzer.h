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

#ifndef INCLUDED_GENALYZER_FFT_VCC_H
#define INCLUDED_GENALYZER_FFT_VCC_H

#include "scopy-gr-util_export.h"
#include <gnuradio/sync_block.h>
#include <cgenalyzer_simplified_beta.h>
#include <cgenalyzer.h>
#include <cstdint>
#include <string>

namespace scopy::grutil {

enum class GenalyzerMode
{
	AUTO = 0,      // Automatic tone detection (current mode)
	FIXED_TONE = 1 // Fixed tone analysis with expected frequency
};

struct SCOPY_GR_UTIL_EXPORT GenalyzerConfig
{
	bool enabled = 0;

	// Common parameters
	GenalyzerMode mode = GenalyzerMode::AUTO;

	// Auto mode parameters
	struct AutoParams
	{
		uint8_t ssb_width = 120;
	} auto_params;

	// Fixed tone mode parameters
	struct FixedToneParams
	{
		double expected_freq = 1000000;
		std::string component_label = "A";
		int harmonic_order = 3;
		int ssb_fundamental = 4;
		int ssb_default = 3;
		double fshift = 0.0;
	} fixed_tone;

	// Helper methods
	bool isAutoMode() const { return mode == GenalyzerMode::AUTO; }
	bool isFixedToneMode() const { return mode == GenalyzerMode::FIXED_TONE; }
};

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
	 * \param do_shift Whether to apply FFT shift (true for complex, false for float)
	 */
	static sptr make(int npts, int qres, int navg, int nfft, GnWindow win, double sample_rate,
			 bool do_shift = true);

	virtual void set_sample_rate(double sample_rate) = 0;
	virtual double sample_rate() const = 0;
	virtual void set_window(GnWindow win) = 0;
	virtual int window() const = 0;
	virtual int navg() const = 0;
	virtual void set_ssb_width(uint8_t ssb_width) = 0; // Deprecated, use set_config
	virtual uint8_t ssb_width() const = 0;		   // Deprecated, use get_config
	virtual void set_config(const GenalyzerConfig &config) = 0;
	virtual GenalyzerConfig get_config() const = 0;
	virtual gn_analysis_results *getGnAnalysis() = 0;

protected:
	genalyzer_fft_vii(const std::string &name, gr::io_signature::sptr input_signature,
			  gr::io_signature::sptr output_signature)
		: gr::sync_block(name, input_signature, output_signature)
	{}
};

} // namespace scopy::grutil

#endif /* INCLUDED_GENALYZER_FFT_VCC_H */
