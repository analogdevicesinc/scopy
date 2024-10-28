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

#include "grsignalsrc.h"
#include "grtopblock.h"
#include "grlog.h"

using namespace scopy::grutil;

GRSignalSrc::GRSignalSrc(QObject *parent)
	: GRProxyBlock(parent)
	, m_waveform(gr::analog::GR_CONST_WAVE)
	, m_sampling_frequency(1000)
	, m_freq(100)
	, m_phase(0)
	, m_offset(0)
{}

GRSignalSrc::~GRSignalSrc() {}

void GRSignalSrc::setWaveform(gr::analog::gr_waveform_t waveform)
{
	m_waveform = waveform;
	if(sig)
		sig->set_waveform(waveform);
}

void GRSignalSrc::setSamplingFreq(double sr)
{
	m_sampling_frequency = sr;
	if(sig)
		sig->set_sampling_freq(m_sampling_frequency);
}
void GRSignalSrc::setFreq(double f)
{
	m_freq = f;
	if(sig)
		sig->set_frequency(m_freq);
}

void GRSignalSrc::setPhase(double ph)
{
	m_phase = ph;
	if(sig)
		sig->set_phase(m_phase);
}

void GRSignalSrc::setAmplitude(double ampl)
{
	m_amplitude = ampl;
	if(sig)
		sig->set_amplitude(m_amplitude);
}

void GRSignalSrc::setOffset(double off)
{
	m_offset = off;
	if(sig)
		sig->set_offset(m_offset);
}

void GRSignalSrc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL) << "Building GRSignalSrc";
	sig = gr::analog::sig_source_f::make(m_sampling_frequency, m_waveform, m_freq, m_amplitude, m_offset, m_phase);
	s2v = gr::blocks::stream_to_vector::make(sizeof(float), top->vlen());
	top->connect(sig, 0, s2v, 0);
	start_blk.append(sig);
	start_blk.append(s2v);
	end_blk = s2v;
}

void GRSignalSrc::destroy_blks(GRTopBlock *top)
{
	end_blk = nullptr;
	sig = nullptr;
	start_blk.clear();
}

#include "moc_grsignalsrc.cpp"
