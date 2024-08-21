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
	QMap<gr::fft::window::win_type, double> m_wincorr_factor;
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
	QMap<gr::fft::window::win_type, double> m_wincorr_factor;
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
