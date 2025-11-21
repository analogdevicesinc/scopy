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

#include "genalyzer.h"
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/moving_average.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/vector_to_stream.h>
#include <gnuradio/fft/fft_v.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/nlog10_ff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_const_v.h>
#include <gnuradio/blocks/float_to_int.h>
#include <gnuradio/blocks/complex_to_float.h>
#include <gnuradio/blocks/vector_source.h>
#include <QMap>
#include <cgenalyzer.h>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRFFTFloatProc : public GRProxyBlock
{
public:
	GRFFTFloatProc(QObject *parent = nullptr);
	void setWindow(gr::fft::window::win_type w);
	void setWindowCorrection(bool b);
	void setPowerOffset(double);
	void setNrBits(int);
	void setSampleRate(double sr);
	void setSigned(bool sig);
	void setNavg(int navg);
	gn_analysis_results *getGnAnalysis();
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	double m_powerOffset;
	int nrBits;
	bool m_windowCorr;
	double m_sr;
	bool m_signed;
	int m_navg;
	gr::blocks::float_to_int::sptr float_to_int_i;
	gr::blocks::add_const_vff::sptr powerOffset;
	genalyzer_fft_vii::sptr genalyzer_fft;

	gr::fft::window::win_type m_fftwindow;
	GRTopBlock *m_top;

private:
	GnWindow convertToGnWindow(gr::fft::window::win_type window_type);
};

class SCOPY_GR_UTIL_EXPORT GRFFTComplexProc : public GRProxyBlock
{
public:
	GRFFTComplexProc(QObject *parent = nullptr);
	void setWindow(gr::fft::window::win_type w);
	void setPowerOffset(double);
	void setWindowCorrection(bool b);
	void setNrBits(int);
	void setSampleRate(double sr);
	void setSigned(bool sig);
	void setNavg(int navg);
	gn_analysis_results *getGnAnalysis();
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	double m_powerOffset;
	int nrBits;
	bool m_windowCorr;
	double m_sr;
	bool m_signed;
	int m_navg;
	gr::blocks::float_to_int::sptr float_to_int_i;
	gr::blocks::float_to_int::sptr float_to_int_q;
	gr::blocks::complex_to_float::sptr complex_to_float;
	gr::blocks::add_const_vff::sptr powerOffset;
	genalyzer_fft_vii::sptr genalyzer_fft;

	gr::fft::window::win_type m_fftwindow;
	GRTopBlock *m_top;

private:
	GnWindow convertToGnWindow(gr::fft::window::win_type window_type);
};

} // namespace scopy::grutil

#endif // GRFFTFLOATPROXY_H
