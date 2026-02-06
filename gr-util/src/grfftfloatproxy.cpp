/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "grfftfloatproxy.h"
#include "grtopblock.h"
#include "qdebug.h"

using namespace scopy::grutil;
GRFFTFloatProc::GRFFTFloatProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANNING;
	m_powerOffset = 0;
	nrBits = 12;
	m_sr = 0;
	m_signed = true;
	m_navg = 1;
	// Initialize with default genalyzer config
	m_genalyzer_config = GenalyzerConfig();
	m_genalyzer_config.auto_params.ssb_width = 120; // Default SSB width
}

void GRFFTFloatProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	Q_EMIT requestRebuild();
}

void GRFFTFloatProc::setPowerOffset(double val)
{
	m_powerOffset = val;
	if(powerOffset) {
		std::vector<float> k;
		for(int i = 0; i < m_top->vlen(); i++) {
			k.push_back(m_powerOffset);
		}
		powerOffset->set_k(k);
	}
}

void GRFFTFloatProc::setNrBits(int v) { nrBits = v; }

void GRFFTFloatProc::setSigned(bool sig) { m_signed = sig; }

void GRFFTFloatProc::setSampleRate(double sr) { m_sr = sr; }

void GRFFTFloatProc::setNavg(int navg)
{
	m_navg = navg;
	requestRebuild();
}

void GRFFTFloatProc::setGenalyzerConfig(const GenalyzerConfig &config)
{
	// Store the configuration locally
	m_genalyzer_config = config;

	// Apply to block if it exists
	if(genalyzer_fft) {
		genalyzer_fft->set_config(config);
	}
}

GenalyzerConfig GRFFTFloatProc::getGenalyzerConfig() const
{
	// Always return the stored configuration
	// It should be in sync with the block's config if the block exists
	return m_genalyzer_config;
}

gn_analysis_results *GRFFTFloatProc::getGnAnalysis()
{
	if(genalyzer_fft) {
		return genalyzer_fft->getGnAnalysis();
	}
	return nullptr;
}

GnWindow GRFFTFloatProc::convertToGnWindow(gr::fft::window::win_type window_type)
{
	switch(window_type) {
	case gr::fft::window::WIN_HANN:
		return GnWindow::GnWindowHann;
	case gr::fft::window::WIN_BLACKMAN_hARRIS:
		return GnWindow::GnWindowBlackmanHarris;
	default:
		return GnWindow::GnWindowNoWindow;
	}
}

void GRFFTFloatProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();

	// Create float to int converter for the input signal
	float_to_int_i = gr::blocks::float_to_int::make(fft_size);

	// Create genalyzer FFT with int32 inputs (using 2 inputs, but Q will be zeros for float mode)
	genalyzer_fft = genalyzer_fft_vii::make(fft_size * m_navg,  // npts - number of points (FFT size * averages)
						nrBits + !m_signed, // qres - quantization resolution for genalyzer
						m_navg,		    // navg - number of averages
						fft_size,	    // nfft - FFT size
						convertToGnWindow(m_fftwindow), // Convert your window type to GnWindow
						m_sr,				// sample rate
						false				// do_shift = false for float mode
	);

	// Apply the stored configuration
	genalyzer_fft->set_config(m_genalyzer_config);

	// Power offset
	std::vector<float> k;
	for(int i = 0; i < fft_size; i++) {
		k.push_back(m_powerOffset);
	}
	powerOffset = gr::blocks::add_const_v<float>::make(k);

	auto zero_source = gr::blocks::vector_source<int32_t>::make(std::vector<int32_t>(fft_size, 0), true, fft_size);

	top->connect(float_to_int_i, 0, genalyzer_fft, 0);
	top->connect(zero_source, 0, genalyzer_fft, 1);
	top->connect(genalyzer_fft, 0, powerOffset, 0);

	start_blk.append(float_to_int_i);
	end_blk = powerOffset;
}

void GRFFTFloatProc::destroy_blks(GRTopBlock *top)
{
	qInfo() << "destroyed grfftfloatproc";
	genalyzer_fft = nullptr;
	float_to_int_i = nullptr;
	powerOffset = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}

GRFFTComplexProc::GRFFTComplexProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANNING;
	m_powerOffset = 0;
	nrBits = 12;
	m_sr = 0;
	m_signed = true;
	m_navg = 1;
	// Initialize with default genalyzer config
	m_genalyzer_config = GenalyzerConfig();
	m_genalyzer_config.auto_params.ssb_width = 120; // Default SSB width
}

void GRFFTComplexProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	Q_EMIT requestRebuild();
}

void GRFFTComplexProc::setPowerOffset(double val)
{
	m_powerOffset = val;
	if(powerOffset) {
		std::vector<float> k;
		for(int i = 0; i < m_top->vlen(); i++) {
			k.push_back(m_powerOffset);
		}
		powerOffset->set_k(k);
	}
}

void GRFFTComplexProc::setNrBits(int v) { nrBits = v; }

void GRFFTComplexProc::setSigned(bool sig) { m_signed = sig; }

void GRFFTComplexProc::setSampleRate(double sr) { m_sr = sr; }

void GRFFTComplexProc::setNavg(int navg)
{
	m_navg = navg;
	requestRebuild();
}

void GRFFTComplexProc::setGenalyzerConfig(const GenalyzerConfig &config)
{
	// Store the configuration locally
	m_genalyzer_config = config;

	// Apply to block if it exists
	if(genalyzer_fft) {
		genalyzer_fft->set_config(config);
	}
}

GenalyzerConfig GRFFTComplexProc::getGenalyzerConfig() const
{
	// Always return the stored configuration
	// It should be in sync with the block's config if the block exists
	return m_genalyzer_config;
}

gn_analysis_results *GRFFTComplexProc::getGnAnalysis()
{
	if(genalyzer_fft) {
		return genalyzer_fft->getGnAnalysis();
	}

	return nullptr;
}
void GRFFTComplexProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();

	// Create conversion blocks for complex to separate I/Q int32 streams
	complex_to_float = gr::blocks::complex_to_float::make(fft_size);
	float_to_int_i = gr::blocks::float_to_int::make(fft_size);
	float_to_int_q = gr::blocks::float_to_int::make(fft_size);

	// Create genalyzer FFT with int32 inputs
	genalyzer_fft = genalyzer_fft_vii::make(fft_size * m_navg,  // npts - number of points (FFT size * averages)
						nrBits + !m_signed, // qres - quantization resolution for genalyzer
						m_navg,		    // navg - number of averages
						fft_size,	    // nfft - FFT size
						convertToGnWindow(m_fftwindow), // Convert your window type to GnWindow
						m_sr,				// sample rate
						true				// do_shift = true for complex mode
	);

	// Apply the stored configuration
	genalyzer_fft->set_config(m_genalyzer_config);

	// Power offset
	std::vector<float> k;
	for(int i = 0; i < fft_size; i++) {
		k.push_back(m_powerOffset);
	}
	powerOffset = gr::blocks::add_const_v<float>::make(k);

	top->connect(complex_to_float, 0, float_to_int_i, 0);
	top->connect(complex_to_float, 1, float_to_int_q, 0);
	top->connect(float_to_int_i, 0, genalyzer_fft, 0);
	top->connect(float_to_int_q, 0, genalyzer_fft, 1);
	top->connect(genalyzer_fft, 0, powerOffset, 0);

	start_blk.append(complex_to_float);
	end_blk = powerOffset;
}

GnWindow GRFFTComplexProc::convertToGnWindow(gr::fft::window::win_type window_type)
{
	switch(window_type) {
	case gr::fft::window::WIN_HANN:
		return GnWindow::GnWindowHann;
	case gr::fft::window::WIN_BLACKMAN_hARRIS:
		return GnWindow::GnWindowBlackmanHarris;
	default:
		return GnWindow::GnWindowNoWindow;
	}
}

void GRFFTComplexProc::destroy_blks(GRTopBlock *top)
{
	genalyzer_fft = nullptr;
	complex_to_float = nullptr;
	float_to_int_i = nullptr;
	float_to_int_q = nullptr;
	powerOffset = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}
