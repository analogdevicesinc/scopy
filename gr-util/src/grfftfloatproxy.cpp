#include "grfftfloatproxy.h"
#include "grtopblock.h"

using namespace scopy::grutil;
GRFFTFloatProc::GRFFTFloatProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANN;
}

void GRFFTFloatProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	/*if(mul)
		mul->set_k(m_scale);*/
}

void GRFFTFloatProc::build_blks(GRTopBlock *top)
{
	auto fft_size = top->vlen();
	auto window = gr::fft::window::build(m_fftwindow, fft_size);

	fft = gr::fft::fft_v<float, true>::make(fft_size, window, true);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);

	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / (fft_size * fft_size), fft_size);

	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);

	top->connect(fft, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);

	start_blk.append(fft);
	end_blk = nlog10;
}

void GRFFTFloatProc::destroy_blks(GRTopBlock *top)
{
	fft = nullptr;
	ctm = nullptr;
	mult_const1 = nullptr;
	nlog10 = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}


GRFFTComplexProc::GRFFTComplexProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANN;
}

void GRFFTComplexProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	/*if(mul)
		mul->set_k(m_scale);*/
}

void GRFFTComplexProc::build_blks(GRTopBlock *top)
{
	auto fft_size = top->vlen();
	auto window = gr::fft::window::build(m_fftwindow, fft_size);

	fft_complex = gr::fft::fft_v<gr_complex, true>::make(fft_size, window, true);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);
	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / ((float)fft_size * (float)fft_size), fft_size);
	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);

	top->connect(fft_complex, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);

	start_blk.append(fft_complex);
	end_blk = nlog10;
}

void GRFFTComplexProc::destroy_blks(GRTopBlock *top)
{
	fft_complex = nullptr;
	ctm = nullptr;
	mult_const1 = nullptr;
	nlog10 = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}
