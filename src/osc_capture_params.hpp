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

#ifndef OSC_CAPTURE_PARAMS_H
#define OSC_CAPTURE_PARAMS_H

#include <vector>

class OscCaptureParams
{
public:
	struct capture_parameters {
		double sampleRate;
		double timePos;
		unsigned long entireBufferSize;
		unsigned long triggerBufferSize;
		unsigned long maxBufferSize;
		std::vector<unsigned long long> availableBufferSizes;
	};

	virtual capture_parameters captureParameters() const = 0;

	virtual void setTimeBase(double secsPerDiv) = 0;
	virtual void setTriggerPos(double pos) = 0;
};

class SymmetricBufferMode: public OscCaptureParams
{
public:
	SymmetricBufferMode();
	~SymmetricBufferMode();

	capture_parameters captureParameters() const;

	void setSampleRates(const std::vector<double>& sampleRates);
	void setEntireBufferMaxSize(unsigned long maxSize);
	void setTriggerBufferMaxSize(unsigned long maxSize);
	void setTimeDivisionCount(int count);

	void setTimeBase(double secsPerDiv);
	void setTriggerPos(double pos);
	void setCustomBufferSize(unsigned long customSize);
	bool isEnhancedMemDepth();
	bool setEnhancedMemDepth(bool);

private:
	void configParamsOnTimeBaseChanged();
	void configParamsOnTriggPosChanged();
	void configParamsOnCustomSizeChanged();

	unsigned long getVisibleBufferSize(double sampleRate)
	{
		return m_timeBase * m_timeDivsCount * // The time span
			sampleRate +
			0.5; // Round the positive value to nearest int
	}

	double getSamplerateFor(unsigned long);

private:
	std::vector<double> m_sampleRates;
	unsigned int m_timeDivsCount;
	unsigned long long m_entireBufferMaxSize;
	unsigned long long m_triggerBufferMaxSize;
	std::vector<unsigned long long> m_availableBufferSizes;

	double m_timeBase;
	double m_triggerPos;

	double m_sampleRate;
	double m_triggPosSR;
	unsigned long m_visibleBufferSize;
	unsigned long m_triggerBufferSize;
	bool m_enhancedMemoryDepth;
};

#endif // OSC_CAPTURE_PARAMS_H
