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
	m_fftwindow = gr::fft::window::WIN_HANN;
	m_powerOffset = 0;
	nrBits = 12;
	m_windowCorr = true;
}

void GRFFTFloatProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	Q_EMIT requestRebuild();
}

void GRFFTFloatProc::setWindowCorrection(bool b)
{
	m_windowCorr = b;
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

void GRFFTFloatProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();

	auto window = gr::fft::window::build(m_fftwindow, fft_size);
	float window_sum = 0;
	for(auto v : window) {
		window_sum += v;
	}
	auto corr = (m_windowCorr) ? window.size() / window_sum : 1;

	fft = gr::fft::fft_v<float, true>::make(fft_size, window, false);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);

	mult_nrbits = gr::blocks::multiply_const_ff::make(1.00 / (1 << nrBits), fft_size);
	mult_wind_corr = gr::blocks::multiply_const_cc::make(gr_complex(corr, corr), fft_size);
	mult_const1 = gr::blocks::multiply_const_ff::make(1.00 / (fft_size * fft_size), fft_size);

	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);
	std::vector<float> k;
	for(int i = 0; i < fft_size; i++) {
		k.push_back(m_powerOffset);
	}

	powerOffset = gr::blocks::add_const_v<float>::make(k);

	top->connect(mult_nrbits, 0, fft, 0);
	top->connect(fft, 0, mult_wind_corr, 0);
	top->connect(mult_wind_corr, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);
	top->connect(nlog10, 0, powerOffset, 0);

	start_blk.append(mult_nrbits);
	end_blk = powerOffset;
}

void GRFFTFloatProc::destroy_blks(GRTopBlock *top)
{

	qInfo() << "destroyed grfftfloatproc";
	mult_nrbits = nullptr;
	fft = nullptr;
	ctm = nullptr;
	mult_wind_corr = nullptr;
	mult_const1 = nullptr;
	nlog10 = nullptr;
	powerOffset = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}

GRFFTComplexProc::GRFFTComplexProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANNING;
	nrBits = 12;
	m_powerOffset = 0;
	m_windowCorr = true;
}

void GRFFTComplexProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	Q_EMIT requestRebuild();

	/*if(mul)
		mul->set_k(m_scale);*/
}

void GRFFTComplexProc::setWindowCorrection(bool b)
{
	m_windowCorr = b;
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

void GRFFTComplexProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();
	std::vector<float> window = gr::fft::window::build(m_fftwindow, fft_size);

	float window_sum = 0;
	for(auto v : window) {
		window_sum += v;
	}
	auto corr = (m_windowCorr) ? window.size() / window_sum : 1;

	mult_nrbits = gr::blocks::multiply_const_cc::make(gr_complex(1.0 / (1 << nrBits), 0), fft_size);
	fft_complex = gr::fft::fft_v<gr_complex, true>::make(fft_size, window, true);

	mult_wind_corr = gr::blocks::multiply_const_cc::make(gr_complex(corr, 0), fft_size);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);
	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / ((float)fft_size * (float)fft_size), fft_size);
	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);

	std::vector<float> k;
	for(int i = 0; i < fft_size; i++) {
		k.push_back(m_powerOffset);
	}

	powerOffset = gr::blocks::add_const_v<float>::make(k);

	top->connect(mult_nrbits, 0, fft_complex, 0);
	top->connect(fft_complex, 0, mult_wind_corr, 0);
	top->connect(mult_wind_corr, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);
	top->connect(nlog10, 0, powerOffset, 0);

	start_blk.append(mult_nrbits);
	end_blk = powerOffset;
}

void GRFFTComplexProc::destroy_blks(GRTopBlock *top)
{
	mult_nrbits = nullptr;
	fft_complex = nullptr;
	ctm = nullptr;
	mult_wind_corr = nullptr;
	mult_const1 = nullptr;
	powerOffset = nullptr;
	nlog10 = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}

GRFFTAvgProc::GRFFTAvgProc(bool complex, QObject *parent)
	: GRProxyBlock(parent)
	, m_size(2)
	, m_complex(complex)
{}

void GRFFTAvgProc::setSize(int size)
{
	m_size = size;
	setEnabled(m_size > 1);
	Q_EMIT requestRebuild();
}

void GRFFTAvgProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();

	if(m_complex) {
		m_avg = gr::blocks::moving_average_cc::make(m_size, gr_complex(1.0f / m_size), 1, fft_size / 2);
	} else {
		m_avg = gr::blocks::moving_average_ff::make(m_size, 1.0f / m_size, 1, fft_size);
	}

	start_blk.append(m_avg);
	end_blk = m_avg;
}

void GRFFTAvgProc::destroy_blks(GRTopBlock *top)
{
	m_avg = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}

int GRFFTAvgProc::size() { return m_size; }
