/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LA_CAPTURE_PARAMS_H
#define LA_CAPTURE_PARAMS_H

#include <vector>
#include "osc_capture_params.hpp"

class LogicAnalyzerSymmetricBufferMode: public OscCaptureParams
{
public:
	LogicAnalyzerSymmetricBufferMode();
	~LogicAnalyzerSymmetricBufferMode();

	void setMaxSampleRate(double value);
	capture_parameters captureParameters() const;
	void setEntireBufferMaxSize(unsigned long maxSize);
	void setTriggerBufferMaxSize(unsigned long maxSize);
	void setTimeDivisionCount(int count);

	void setTimeBase(double secsPerDiv);
	void setTriggerPos(double pos);

private:
	void configParamsOnTimeBaseChanged();
	void configParamsOnTriggPosChanged();

	unsigned long getVisibleBufferSize(double sampleRate)
	{
		return m_timeBase * m_timeDivsCount * // The time span
			sampleRate +
			0.5; // Round the positive value to nearest int
	}


private:
	double m_maxSampleRate;
	int m_current_divider;

	unsigned int m_timeDivsCount;
	long long m_entireBufferMaxSize;
	long long m_triggerBufferMaxSize;

	double m_timeBase;
	double m_triggerPos;

	double m_sampleRate;
	double m_triggPosSR;
	unsigned long m_visibleBufferSize;
	long long m_triggerBufferSize;
};

#endif // LA_CAPTURE_PARAMS_H
