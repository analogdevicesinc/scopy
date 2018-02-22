/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "osc_capture_params.hpp"

#include <algorithm>
#include <QtGlobal>

/*
 * Class SymmetricBufferMode implementation
 */
SymmetricBufferMode::SymmetricBufferMode() :
	m_sampleRates(),
	m_entireBufferMaxSize(0),
	m_triggerBufferMaxSize(0),
	m_timeBase(0.0),
	m_triggerPos(0.0),
	m_sampleRate(0.0),
	m_triggPosSR(0.0),
	m_visibleBufferSize(0),
	m_triggerBufferSize(0),
	m_enhancedMemoryDepth(false)
{
}

SymmetricBufferMode::~SymmetricBufferMode()
{
}

SymmetricBufferMode::capture_parameters
SymmetricBufferMode::captureParameters() const
{
	struct capture_parameters params;
	double sampleRate;
	unsigned long bufferSize;

	// If trigger position altered the sample rate, return the altered one
	sampleRate = qMin(m_sampleRate, m_triggPosSR);

	// If trigger position altered the buffer size, return the altered one
	bufferSize = qMax(m_visibleBufferSize, m_triggerBufferSize);

	params.entireBufferSize = bufferSize;
	params.sampleRate = sampleRate;
	params.timePos = m_triggerPos;
	params.triggerBufferSize = m_triggerBufferSize;
	params.availableBufferSizes = m_availableBufferSizes;
	params.maxBufferSize = m_triggerBufferSize;

	return params;
}

void SymmetricBufferMode::setSampleRates(const std::vector<double>& sampleRates)
{
	m_sampleRates = sampleRates;
	std::sort(m_sampleRates.begin(), m_sampleRates.end());
}

void SymmetricBufferMode::setEntireBufferMaxSize(unsigned long maxSize)
{
	m_entireBufferMaxSize = maxSize;
}

void SymmetricBufferMode::setTriggerBufferMaxSize(unsigned long maxSize)
{
	m_triggerBufferMaxSize = maxSize;
}

void SymmetricBufferMode::setTimeDivisionCount(int count)
{
	m_timeDivsCount = count;
}

void SymmetricBufferMode::setTimeBase(double secsPerDiv)
{
	if ((m_timeBase != secsPerDiv) || m_enhancedMemoryDepth) {
		m_timeBase = secsPerDiv;
		configParamsOnTimeBaseChanged();
	}
}

void SymmetricBufferMode::setCustomBufferSize(unsigned long customSize)
{
	m_visibleBufferSize = customSize;
	configParamsOnCustomSizeChanged();
}

bool SymmetricBufferMode::isEnhancedMemDepth()
{
	return m_enhancedMemoryDepth;
}

bool SymmetricBufferMode::setEnhancedMemDepth(bool val)
{
	m_enhancedMemoryDepth = val;
}

void SymmetricBufferMode::setTriggerPos(double pos)
{
	if (m_triggerPos != pos) {
		m_triggerPos = pos;
		configParamsOnTriggPosChanged();
	}
}

void SymmetricBufferMode::configParamsOnTimeBaseChanged()
{
	double sampleRate;

	// Get highest sample rate
	auto ratesIt = m_sampleRates.rbegin();
	sampleRate = *ratesIt;

	m_enhancedMemoryDepth = false;
	m_availableBufferSizes.clear();

	unsigned long bufferSize = getVisibleBufferSize(sampleRate);
	while ((m_triggerBufferMaxSize < bufferSize) && (ratesIt !=
			m_sampleRates.rend() - 1)) {
		if (bufferSize <= m_entireBufferMaxSize) {
			m_availableBufferSizes.insert(m_availableBufferSizes.begin(), bufferSize);
		}
		sampleRate = *(++ratesIt);
		bufferSize = getVisibleBufferSize(sampleRate);
	}
	if (bufferSize <= m_entireBufferMaxSize) {
		m_availableBufferSizes.insert(m_availableBufferSizes.begin(), bufferSize);
	}

	// The zero value of the trigger position (seconds) starts at the middle
	// of the screen while the raw trigger position starts from the left
	// edge of the plot
	long long triggOffset = bufferSize / 2;

	// Make sure trigg pos stays inside the boundaries of the entire buffer
	long long triggPosInBuffer = qRound64(m_triggerPos * sampleRate) +
		triggOffset;

	double trigBuffSize = triggPosInBuffer;
	if (triggPosInBuffer < 0) {
		bufferSize = qRound64(qAbs(m_triggerPos) * sampleRate) +
			triggOffset;

		if (bufferSize > m_entireBufferMaxSize) {
			bufferSize = m_entireBufferMaxSize;
			triggPosInBuffer = -m_entireBufferMaxSize +
				2 * triggOffset;
		}

		trigBuffSize = 0;
	} else if (triggPosInBuffer > m_triggerBufferMaxSize) {
		triggPosInBuffer = m_triggerBufferMaxSize;
		trigBuffSize = triggPosInBuffer;
	}

	m_visibleBufferSize = bufferSize;
	m_triggerBufferSize = trigBuffSize;
	m_triggerPos = (triggPosInBuffer - triggOffset) / sampleRate;
	m_sampleRate = sampleRate;
	m_triggPosSR = sampleRate;
}

double SymmetricBufferMode::getSamplerateFor(unsigned long buffersize)
{
	double desiredSamplrate = buffersize / (m_timeBase * m_timeDivsCount);
	auto ratesIt = m_sampleRates.begin();
	double sampleRate = *ratesIt;

	while ( sampleRate < desiredSamplrate && (ratesIt != m_sampleRates.end() - 1)) {
		sampleRate = *(++ratesIt);
	}
	return sampleRate;
}

void SymmetricBufferMode::configParamsOnCustomSizeChanged()
{
	m_sampleRate = getSamplerateFor(m_visibleBufferSize);
	m_triggerBufferSize = m_triggerBufferMaxSize;
	m_triggPosSR = m_sampleRate;
	/* Move trigger to left such that we have 8192 before the trigger */
	m_triggerPos = (m_timeBase * m_timeDivsCount / 2) - (m_triggerBufferMaxSize/m_sampleRate);
	m_enhancedMemoryDepth = true;
}


void SymmetricBufferMode::configParamsOnTriggPosChanged()
{
	if (m_enhancedMemoryDepth)
		return;

	m_visibleBufferSize = getVisibleBufferSize(m_sampleRate);

	// The zero value of the trigger position (seconds) starts at the middle
	// of the screen while the raw trigger position starts from the left
	// edge of the plot
	long long triggOffset = m_visibleBufferSize / 2;

	long long triggPosInBuffer = qRound64(m_triggerPos * m_sampleRate) +
		triggOffset;

	m_triggPosSR = m_sampleRate;

	if (triggPosInBuffer < 0) {
		m_triggerBufferSize = 0;
		unsigned long delaySamples = -triggPosInBuffer;
		double bufferSize = (m_timeBase * m_timeDivsCount) *
			m_sampleRate + delaySamples;

		// Limit buffer size and recalculate trigger position
		if (bufferSize > m_entireBufferMaxSize) {
			bufferSize = m_entireBufferMaxSize;
			long long triggPosInBuffer = -m_entireBufferMaxSize +
				2 * triggOffset;
			m_triggerPos = (triggPosInBuffer - triggOffset) /
				m_sampleRate;
		}

		m_visibleBufferSize = bufferSize;

		return;
	}

	// Find the highest sample rate that can be set based on trigger pos
	auto ratesIt = find(m_sampleRates.rbegin(), m_sampleRates.rend(),
		m_sampleRate);
	while ((triggPosInBuffer > m_triggerBufferMaxSize) && (ratesIt <
			m_sampleRates.rend() - 1)) {
		m_triggPosSR = *(++ratesIt);
		// New sample rate -> new buffer size
		m_visibleBufferSize = getVisibleBufferSize(m_triggPosSR);
		triggOffset = m_visibleBufferSize / 2;
		triggPosInBuffer = qRound64(m_triggerPos * m_triggPosSR) +
			triggOffset;
	}
	m_triggerBufferSize = triggPosInBuffer;
}
