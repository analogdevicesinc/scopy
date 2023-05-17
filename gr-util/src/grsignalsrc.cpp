#include "grsignalsrc.h"
#include "grlog.h"

using namespace scopy::grutil;

GRSignalSrc::GRSignalSrc(QObject *parent) : GRProxyBlock(parent) {}

GRSignalSrc::~GRSignalSrc()
{

}

void GRSignalSrc::setWaveform(gr::analog::gr_waveform_t waveform)
{
	m_waveform = waveform;
	if(sig)
		sig->set_waveform(waveform);
}

void GRSignalSrc::setSamplingFreq(double sr) {
	m_sampling_frequency = sr;
	if(sig)
		sig->set_sampling_freq(m_sampling_frequency);
}
void GRSignalSrc::setFreq(double f) {
	m_freq = f;
	if(sig)
		sig->set_frequency(m_freq);
}

void GRSignalSrc::setPhase(double ph) {
	m_phase = ph;
	if(sig)
		sig->set_phase(m_phase);
}

void GRSignalSrc::setAmplitude(double ampl) {
	m_amplitude = ampl;
	if(sig)
		sig->set_amplitude(m_amplitude);
}

void GRSignalSrc::setOffset(double off) {
	m_offset = off;
	if(sig)
		sig->set_offset(m_offset);
}

void GRSignalSrc::build_blks(GRTopBlock *top) {
	qDebug(SCOPY_GR_UTIL)<<"Building GRSignalSrc";
	sig = gr::analog::sig_source_f::make(m_sampling_frequency, m_waveform, m_freq,m_amplitude,m_offset,m_phase);
	start_blk.append(sig);
	end_blk = sig;
}

void GRSignalSrc::destroy_blks(GRTopBlock *top) {
	end_blk = nullptr;
	sig = nullptr;
	start_blk.clear();

}
