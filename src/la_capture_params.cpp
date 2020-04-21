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

#include "la_capture_params.hpp"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

LogicAnalyzerSymmetricBufferMode::LogicAnalyzerSymmetricBufferMode()
	: m_maxSampleRate(100000000)
	, m_current_divider(1)
	, m_timeDivsCount(10)
	, m_entireBufferMaxSize(0)
	, m_triggerBufferMaxSize(0)
	, m_timeBase(0.0)
	, m_triggerPos(0.0)
	, m_sampleRate(0.0)
	, m_triggPosSR(0.0)
	, m_visibleBufferSize(0)
	, m_triggerBufferSize(0) {}

LogicAnalyzerSymmetricBufferMode::capture_parameters
LogicAnalyzerSymmetricBufferMode::captureParameters() const {
	struct capture_parameters params;
	double sampleRate;
	unsigned long bufferSize;
	long long bufferStartingPoint;

	// If trigger position altered the sample rate, return the altered one
	sampleRate = qMin(m_sampleRate, m_triggPosSR);

	// If trigger position altered the buffer size, return the altered one

	if (m_triggerBufferSize >= 0) {
		bufferSize = qMax(m_visibleBufferSize,
				  (unsigned long)m_triggerBufferSize);
	} else {
		bufferSize = m_visibleBufferSize;
	}

	bufferStartingPoint =
		(m_triggerBufferSize < 0) ? 0 : m_triggerBufferSize;

	params.entireBufferSize = bufferSize;
	params.sampleRate = sampleRate;
	params.timePos = m_triggerPos;
	params.triggerBufferSize = m_triggerBufferSize;
	params.maxBufferSize = m_triggerBufferSize;
	params.dataStartingPoint = bufferStartingPoint;

	return params;
}

LogicAnalyzerSymmetricBufferMode::~LogicAnalyzerSymmetricBufferMode() {}

void LogicAnalyzerSymmetricBufferMode::setMaxSampleRate(double value) {
	m_maxSampleRate = value;
}

void LogicAnalyzerSymmetricBufferMode::setEntireBufferMaxSize(
	unsigned long maxSize) {
	m_entireBufferMaxSize = maxSize;
}

void LogicAnalyzerSymmetricBufferMode::setTriggerBufferMaxSize(
	unsigned long maxSize) {
	m_triggerBufferMaxSize = maxSize;
}

void LogicAnalyzerSymmetricBufferMode::setTimeDivisionCount(int count) {
	m_timeDivsCount = count;
}

void LogicAnalyzerSymmetricBufferMode::setTimeBase(double secsPerDiv) {
	if (m_timeBase != secsPerDiv) {
		m_timeBase = secsPerDiv;
		configParamsOnTimeBaseChanged();
	}
}

void LogicAnalyzerSymmetricBufferMode::setTriggerPos(double pos) {
	if (m_triggerPos != pos) {
		m_triggerPos = pos;
		configParamsOnTriggPosChanged();
	}
}

void LogicAnalyzerSymmetricBufferMode::configParamsOnTimeBaseChanged() {
	double sampleRate;

	// Get highest sample rate
	int sr_divider = 1;
	sampleRate = m_maxSampleRate / sr_divider;

	long bufferSize = getVisibleBufferSize(sampleRate);
	while ((m_triggerBufferMaxSize < bufferSize) &&
	       (sr_divider < m_maxSampleRate)) {
		sr_divider++;
		sampleRate = ceil(m_maxSampleRate / sr_divider);
		bufferSize = getVisibleBufferSize(sampleRate);
	}

	// The zero value of the trigger position (seconds) starts at the middle
	// of the screen while the raw trigger position starts from the left
	// edge of the plot
	long long triggOffset = bufferSize / 2;

	// Make sure trigg pos stays inside the boundaries of the entire buffer
	long long triggPosInBuffer =
		qRound64(m_triggerPos * sampleRate) + triggOffset;

	long long trigBuffSize = triggPosInBuffer;
	if (triggPosInBuffer < 0) {
		bufferSize =
			qRound64(qAbs(m_triggerPos) * sampleRate) + triggOffset;

		if (bufferSize > m_entireBufferMaxSize) {
			trigBuffSize = -(bufferSize - m_entireBufferMaxSize);
			bufferSize = m_entireBufferMaxSize;
		} else {
			trigBuffSize = 0;
		}
	} else if (triggPosInBuffer > m_triggerBufferMaxSize) {
		triggPosInBuffer = m_triggerBufferMaxSize;
		trigBuffSize = triggPosInBuffer;
	}

	m_visibleBufferSize = bufferSize;
	m_triggerBufferSize = trigBuffSize;
	m_triggerPos = (triggPosInBuffer - triggOffset) / sampleRate;
	m_sampleRate = sampleRate;
	m_triggPosSR = sampleRate;
	m_current_divider = sr_divider;
}

void LogicAnalyzerSymmetricBufferMode::configParamsOnTriggPosChanged() {
	m_visibleBufferSize = getVisibleBufferSize(m_sampleRate);

	// The zero value of the trigger position (seconds) starts at the middle
	// of the screen while the raw trigger position starts from the left
	// edge of the plot
	long long triggOffset = m_visibleBufferSize / 2;
	double bufferSize;

	long long triggPosInBuffer =
		qRound64(m_triggerPos * m_sampleRate) + triggOffset;

	m_triggPosSR = m_sampleRate;

	if (triggPosInBuffer < 0) {
		unsigned long delaySamples = -triggPosInBuffer;
		bufferSize = (m_timeBase * m_timeDivsCount) * m_sampleRate +
			delaySamples;
		m_triggerBufferSize = 0;

		if (bufferSize > m_entireBufferMaxSize) {
			m_triggerBufferSize =
				-(bufferSize - m_entireBufferMaxSize);
			bufferSize = m_entireBufferMaxSize;
		}
		m_visibleBufferSize = bufferSize;
		return;
	}

	// Get highest sample rate
	int sr_divider = m_current_divider;

	while ((triggPosInBuffer > m_triggerBufferMaxSize) &&
	       (sr_divider < m_maxSampleRate)) {
		sr_divider++;
		m_triggPosSR = m_maxSampleRate / sr_divider;
		// New sample rate -> new buffer size
		m_visibleBufferSize = getVisibleBufferSize(m_triggPosSR);
		triggOffset = m_visibleBufferSize / 2;
		triggPosInBuffer =
			qRound64(m_triggerPos * m_triggPosSR) + triggOffset;
	}
	m_triggerBufferSize = triggPosInBuffer;
}
