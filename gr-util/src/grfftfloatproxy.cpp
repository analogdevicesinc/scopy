#include "grfftfloatproxy.h"
#include "grtopblock.h"
#include "qdebug.h"

using namespace scopy::grutil;
GRFFTFloatProc::GRFFTFloatProc(QObject *parent)
	: GRProxyBlock(parent)
{
	m_fftwindow = gr::fft::window::WIN_HANN;
	m_powerOffset = 0;
}

void GRFFTFloatProc::setWindow(gr::fft::window::win_type w)
{
	m_fftwindow = w;
	/*if(mul)
		mul->set_k(m_scale);*/
}

void GRFFTFloatProc::setPowerOffset(double val) {
	m_powerOffset = val;
	if(powerOffset) {
		std::vector<float> k;
		for(int i = 0;i<m_top->vlen();i++) {
			k.push_back(m_powerOffset);
		}
		powerOffset->set_k(k);
	}
}

void GRFFTFloatProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();

	auto window = gr::fft::window::build(m_fftwindow, fft_size);

	fft = gr::fft::fft_v<float, true>::make(fft_size, window, false);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);

	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / (fft_size * fft_size), fft_size);

	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);
	std::vector<float> k;
	for(int i = 0;i<fft_size;i++) {
		k.push_back(m_powerOffset);
	}

	powerOffset = gr::blocks::add_const_v<float>::make(k);

	top->connect(fft, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);
	top->connect(nlog10, 0, powerOffset, 0);

	start_blk.append(fft);
	end_blk = powerOffset;
}

void GRFFTFloatProc::destroy_blks(GRTopBlock *top)
{

	qInfo()<<"destroyed grfftfloatproc";
	fft = nullptr;
	ctm = nullptr;
	mult_const1 = nullptr;
	nlog10 = nullptr;
	powerOffset = nullptr;
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

void GRFFTComplexProc::setPowerOffset(double val) {
	m_powerOffset = val;
	if(powerOffset) {
		std::vector<float> k;
		for(int i = 0;i<m_top->vlen();i++) {
			k.push_back(m_powerOffset);
		}
		powerOffset->set_k(k);
	}
}


void GRFFTComplexProc::build_blks(GRTopBlock *top)
{
	m_top = top;
	auto fft_size = top->vlen();
	auto window = gr::fft::window::build(m_fftwindow, fft_size);

	fft_complex = gr::fft::fft_v<gr_complex, true>::make(fft_size, window, true);
	ctm = gr::blocks::complex_to_mag_squared::make(fft_size);
	mult_const1 = gr::blocks::multiply_const_ff::make(1.0 / ((float)fft_size * (float)fft_size), fft_size);
	nlog10 = gr::blocks::nlog10_ff::make(10.0, fft_size);

	std::vector<float> k;
	for(int i = 0;i<fft_size;i++) {
		k.push_back(m_powerOffset);
	}

	powerOffset = gr::blocks::add_const_v<float>::make(k);

	top->connect(fft_complex, 0, ctm, 0);
	top->connect(ctm, 0, mult_const1, 0);
	top->connect(mult_const1, 0, nlog10, 0);
	top->connect(nlog10,0,powerOffset,0);

	start_blk.append(fft_complex);
	end_blk = powerOffset;
}

void GRFFTComplexProc::destroy_blks(GRTopBlock *top)
{
	fft_complex = nullptr;
	ctm = nullptr;
	mult_const1 = nullptr;
	powerOffset = nullptr;
	nlog10 = nullptr;
	start_blk.clear();
	end_blk = nullptr;
}
