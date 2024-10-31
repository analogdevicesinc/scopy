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

#ifndef GRSIGNALSRC_H
#define GRSIGNALSRC_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/stream_to_vector.h>

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
	gr::blocks::stream_to_vector::sptr s2v;
	double m_sampling_frequency;
	double m_freq;
	double m_phase;
	double m_amplitude;
	double m_offset;
};
} // namespace scopy::grutil
#endif // GRSIGNALSRC_H
