#ifndef GRSIGNALSRC_H
#define GRSIGNALSRC_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/analog/sig_source.h>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRSignalSrc : public GRProxyBlock
{
	Q_OBJECT
public:
	GRSignalSrc(QObject *parent = nullptr);
	~GRSignalSrc();
	void setWaveform(gr::analog::gr_waveform_t waveform);
	void setSamplingFreq(double sr);
	void setFreq(double f);
	void setPhase(double ph);
	void setAmplitude(double ampl);
	void setOffset(double off);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	gr::analog::sig_source_f::sptr sig;
	gr::analog::gr_waveform_t m_waveform;
	double m_sampling_frequency;
	double m_freq;
	double m_phase;
	double m_amplitude;
	double m_offset;
};
} // namespace scopy::grutil
#endif // GRSIGNALSRC_H
