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

#ifndef GRFFTPROC_H
#define GRFFTPROC_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_const_v.h>
#include <QMap>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRFFTFloatProc : public GRProxyBlock
{
public:
	GRFFTFloatProc(QObject *parent = nullptr);
	void setWindow(gr::fft::window::win_type w);
	void setWindowCorrection(bool b);
	void setPowerOffset(double);
	void setNrBits(int);
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	double m_powerOffset;
	int nrBits;
	gr::fft::fft_v<float, true>::sptr fft;
	bool m_windowCorr;
	gr::blocks::multiply_const_ff::sptr mult_nrbits;
	gr::blocks::complex_to_mag_squared::sptr ctm;
	gr::blocks::multiply_const_cc::sptr mult_wind_corr;
	gr::blocks::multiply_const_ff::sptr mult_const1;
	gr::blocks::nlog10_ff::sptr nlog10;
	gr::blocks::add_const_vff::sptr powerOffset;

	gr::fft::window::win_type m_fftwindow;
	GRTopBlock *m_top;
};

class SCOPY_GR_UTIL_EXPORT GRFFTComplexProc : public GRProxyBlock
{
public:
	GRFFTComplexProc(QObject *parent = nullptr);
	void setWindow(gr::fft::window::win_type w);
	void setPowerOffset(double);
	void setWindowCorrection(bool b);
	void setNrBits(int);
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	double m_powerOffset;
	int nrBits;
	bool m_windowCorr;
	gr::fft::fft_v<gr_complex, true>::sptr fft_complex;
	gr::blocks::multiply_const_cc::sptr mult_nrbits;
	gr::blocks::complex_to_mag_squared::sptr ctm;
	gr::blocks::multiply_const_cc::sptr mult_wind_corr;
	gr::blocks::multiply_const_ff::sptr mult_const1;
	gr::blocks::nlog10_ff::sptr nlog10;
	gr::blocks::add_const_vff::sptr powerOffset;

	gr::fft::window::win_type m_fftwindow;
	GRTopBlock *m_top;
};
} // namespace scopy::grutil

#endif // GRFFTFLOATPROXY_H
