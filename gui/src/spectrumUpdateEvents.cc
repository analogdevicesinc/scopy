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

#ifndef SPECTRUM_UPDATE_EVENTS_C
#define SPECTRUM_UPDATE_EVENTS_C

#include <scopy/gui/spectrumUpdateEvents.h>

using namespace scopy::gui;

SpectrumUpdateEvent::SpectrumUpdateEvent(const float* fftPoints, const uint64_t numFFTDataPoints,
					 const double* realTimeDomainPoints, const double* imagTimeDomainPoints,
					 const uint64_t numTimeDomainDataPoints,
					 const gr::high_res_timer_type dataTimestamp, const bool repeatDataFlag,
					 const bool lastOfMultipleUpdateFlag,
					 const gr::high_res_timer_type generatedTimestamp, const int droppedFFTFrames)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numFFTDataPoints < 1) {
		m_numFFTDataPoints = 1;
	} else {
		m_numFFTDataPoints = numFFTDataPoints;
	}

	if (numTimeDomainDataPoints < 1) {
		m_numTimeDomainDataPoints = 1;
	} else {
		m_numTimeDomainDataPoints = numTimeDomainDataPoints;
	}

	m_fftPoints = new float[m_numFFTDataPoints];
	m_fftPoints[0] = 0;
	memcpy(m_fftPoints, fftPoints, numFFTDataPoints * sizeof(float));

	m_realDataTimeDomainPoints = new double[m_numTimeDomainDataPoints];
	memset(m_realDataTimeDomainPoints, 0x0, m_numTimeDomainDataPoints * sizeof(double));
	if (numTimeDomainDataPoints > 0) {
		memcpy(m_realDataTimeDomainPoints, realTimeDomainPoints, numTimeDomainDataPoints * sizeof(double));
	}

	m_imagDataTimeDomainPoints = new double[m_numTimeDomainDataPoints];
	memset(m_imagDataTimeDomainPoints, 0x0, m_numTimeDomainDataPoints * sizeof(double));
	if (numTimeDomainDataPoints > 0) {
		memcpy(m_imagDataTimeDomainPoints, imagTimeDomainPoints, numTimeDomainDataPoints * sizeof(double));
	}
	m_dataTimestamp = dataTimestamp;
	m_repeatDataFlag = repeatDataFlag;
	m_lastOfMultipleUpdateFlag = lastOfMultipleUpdateFlag;
	m_eventGeneratedTimestamp = generatedTimestamp;
	m_droppedFFTFrames = droppedFFTFrames;
}

SpectrumUpdateEvent::~SpectrumUpdateEvent()
{
	delete[] m_fftPoints;
	delete[] m_realDataTimeDomainPoints;
	delete[] m_imagDataTimeDomainPoints;
}

const float* SpectrumUpdateEvent::getFFTPoints() const { return m_fftPoints; }

const double* SpectrumUpdateEvent::getRealTimeDomainPoints() const { return m_realDataTimeDomainPoints; }

const double* SpectrumUpdateEvent::getImagTimeDomainPoints() const { return m_imagDataTimeDomainPoints; }

uint64_t SpectrumUpdateEvent::getNumFFTDataPoints() const { return m_numFFTDataPoints; }

uint64_t SpectrumUpdateEvent::getNumTimeDomainDataPoints() const { return m_numTimeDomainDataPoints; }

gr::high_res_timer_type SpectrumUpdateEvent::getDataTimestamp() const { return m_dataTimestamp; }

bool SpectrumUpdateEvent::getRepeatDataFlag() const { return m_repeatDataFlag; }

bool SpectrumUpdateEvent::getLastOfMultipleUpdateFlag() const { return m_lastOfMultipleUpdateFlag; }

gr::high_res_timer_type SpectrumUpdateEvent::getEventGeneratedTimestamp() const { return m_eventGeneratedTimestamp; }

int SpectrumUpdateEvent::getDroppedFFTFrames() const { return m_droppedFFTFrames; }

SpectrumWindowCaptionEvent::SpectrumWindowCaptionEvent(const QString& newLbl)
	: QEvent(QEvent::Type(SpectrumWindowCaptionEventType))
{
	m_labelString = newLbl;
}

SpectrumWindowCaptionEvent::~SpectrumWindowCaptionEvent() {}

QString SpectrumWindowCaptionEvent::getLabel() { return m_labelString; }

SpectrumWindowResetEvent::SpectrumWindowResetEvent()
	: QEvent(QEvent::Type(SpectrumWindowResetEventType))
{}

SpectrumWindowResetEvent::~SpectrumWindowResetEvent() {}

SpectrumFrequencyRangeEvent::SpectrumFrequencyRangeEvent(const double centerFreq, const double startFreq,
							 const double stopFreq)
	: QEvent(QEvent::Type(SpectrumFrequencyRangeEventType))
{
	m_centerFrequency = centerFreq;
	m_startFrequency = startFreq;
	m_stopFrequency = stopFreq;
}

SpectrumFrequencyRangeEvent::~SpectrumFrequencyRangeEvent() {}

double SpectrumFrequencyRangeEvent::GetCenterFrequency() const { return m_centerFrequency; }

double SpectrumFrequencyRangeEvent::GetStartFrequency() const { return m_startFrequency; }

double SpectrumFrequencyRangeEvent::GetStopFrequency() const { return m_stopFrequency; }

/***************************************************************************/

TimeUpdateEvent::TimeUpdateEvent(const std::vector<double*>& timeDomainPoints, const uint64_t numTimeDomainDataPoints,
				 const std::vector<std::vector<gr::tag_t>>& tags)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numTimeDomainDataPoints < 1) {
		m_numTimeDomainDataPoints = 1;
	} else {
		m_numTimeDomainDataPoints = numTimeDomainDataPoints;
	}

	m_nplots = timeDomainPoints.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_dataTimeDomainPoints.push_back(new double[m_numTimeDomainDataPoints]);
		if (numTimeDomainDataPoints > 0) {
			memcpy(m_dataTimeDomainPoints[i], timeDomainPoints[i],
			       m_numTimeDomainDataPoints * sizeof(double));
		}
	}

	m_tags = tags;
}

TimeUpdateEvent::~TimeUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_dataTimeDomainPoints[i];
	}
}

const std::vector<double*> TimeUpdateEvent::getTimeDomainPoints() const { return m_dataTimeDomainPoints; }

uint64_t TimeUpdateEvent::getNumTimeDomainDataPoints() const { return m_numTimeDomainDataPoints; }

const std::vector<std::vector<gr::tag_t>> TimeUpdateEvent::getTags() const { return m_tags; }

/***************************************************************************/

IdentifiableTimeUpdateEvent::IdentifiableTimeUpdateEvent(const std::vector<double*>& timeDomainPoints,
							 const uint64_t numTimeDomainDataPoints,
							 const std::vector<std::vector<gr::tag_t>>& tags,
							 const std::string& senderName)
	: TimeUpdateEvent(timeDomainPoints, numTimeDomainDataPoints, tags)
	, m_senderName(senderName)
{}

IdentifiableTimeUpdateEvent::~IdentifiableTimeUpdateEvent() {}

std::string IdentifiableTimeUpdateEvent::senderName() { return m_senderName; }

/***************************************************************************/

FreqUpdateEvent::FreqUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numDataPoints < 1) {
		m_numDataPoints = 1;
	} else {
		m_numDataPoints = numDataPoints;
	}

	m_nplots = dataPoints.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_dataPoints.push_back(new double[m_numDataPoints]);
		if (numDataPoints > 0) {
			memcpy(m_dataPoints[i], dataPoints[i], m_numDataPoints * sizeof(double));
		}
	}
}

FreqUpdateEvent::~FreqUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_dataPoints[i];
	}
}

const std::vector<double*> FreqUpdateEvent::getPoints() const { return m_dataPoints; }

uint64_t FreqUpdateEvent::getNumDataPoints() const { return m_numDataPoints; }

SetFreqEvent::SetFreqEvent(const double centerFreq, const double bandwidth)
	: QEvent(QEvent::Type(SpectrumFrequencyRangeEventType))
{
	m_centerFrequency = centerFreq;
	m_bandwidth = bandwidth;
}

SetFreqEvent::~SetFreqEvent() {}

double SetFreqEvent::getCenterFrequency() const { return m_centerFrequency; }

double SetFreqEvent::getBandwidth() const { return m_bandwidth; }

/***************************************************************************/

ConstUpdateEvent::ConstUpdateEvent(const std::vector<double*>& realDataPoints,
				   const std::vector<double*>& imagDataPoints, const uint64_t numDataPoints)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numDataPoints < 1) {
		m_numDataPoints = 1;
	} else {
		m_numDataPoints = numDataPoints;
	}

	m_nplots = realDataPoints.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_realDataPoints.push_back(new double[m_numDataPoints]);
		m_imagDataPoints.push_back(new double[m_numDataPoints]);
		if (numDataPoints > 0) {
			memcpy(m_realDataPoints[i], realDataPoints[i], m_numDataPoints * sizeof(double));
			memcpy(m_imagDataPoints[i], imagDataPoints[i], m_numDataPoints * sizeof(double));
		}
	}
}

ConstUpdateEvent::~ConstUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_realDataPoints[i];
		delete[] m_imagDataPoints[i];
	}
}

const std::vector<double*> ConstUpdateEvent::getRealPoints() const { return m_realDataPoints; }

const std::vector<double*> ConstUpdateEvent::getImagPoints() const { return m_imagDataPoints; }

uint64_t ConstUpdateEvent::getNumDataPoints() const { return m_numDataPoints; }

/***************************************************************************/

WaterfallUpdateEvent::WaterfallUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints,
					   const gr::high_res_timer_type dataTimestamp)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numDataPoints < 1) {
		m_numDataPoints = 1;
	} else {
		m_numDataPoints = numDataPoints;
	}

	m_nplots = dataPoints.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_dataPoints.push_back(new double[m_numDataPoints]);
		if (numDataPoints > 0) {
			memcpy(m_dataPoints[i], dataPoints[i], m_numDataPoints * sizeof(double));
		}
	}

	m_dataTimestamp = dataTimestamp;
}

WaterfallUpdateEvent::~WaterfallUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_dataPoints[i];
	}
}

const std::vector<double*> WaterfallUpdateEvent::getPoints() const { return m_dataPoints; }

uint64_t WaterfallUpdateEvent::getNumDataPoints() const { return m_numDataPoints; }

gr::high_res_timer_type WaterfallUpdateEvent::getDataTimestamp() const { return m_dataTimestamp; }

/***************************************************************************/

TimeRasterUpdateEvent::TimeRasterUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (numDataPoints < 1) {
		m_numDataPoints = 1;
	} else {
		m_numDataPoints = numDataPoints;
	}

	m_nplots = dataPoints.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_dataPoints.push_back(new double[m_numDataPoints]);
		if (numDataPoints > 0) {
			memcpy(m_dataPoints[i], dataPoints[i], m_numDataPoints * sizeof(double));
		}
	}
}

TimeRasterUpdateEvent::~TimeRasterUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_dataPoints[i];
	}
}

const std::vector<double*> TimeRasterUpdateEvent::getPoints() const { return m_dataPoints; }

uint64_t TimeRasterUpdateEvent::getNumDataPoints() const { return m_numDataPoints; }

/***************************************************************************/

HistogramUpdateEvent::HistogramUpdateEvent(const std::vector<double*>& points, const uint64_t npoints)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
{
	if (npoints < 1) {
		m_npoints = 1;
	} else {
		m_npoints = npoints;
	}

	m_nplots = points.size();
	for (size_t i = 0; i < m_nplots; i++) {
		m_points.push_back(new double[m_npoints]);
		if (npoints > 0) {
			memcpy(m_points[i], points[i], m_npoints * sizeof(double));
		}
	}
}

HistogramUpdateEvent::~HistogramUpdateEvent()
{
	for (size_t i = 0; i < m_nplots; i++) {
		delete[] m_points[i];
	}
}

const std::vector<double*> HistogramUpdateEvent::getDataPoints() const { return m_points; }

uint64_t HistogramUpdateEvent::getNumDataPoints() const { return m_npoints; }

/***************************************************************************/

NumberUpdateEvent::NumberUpdateEvent(const std::vector<float>& samples)
	: QEvent(QEvent::Type(SpectrumUpdateEventType))
	, m_samples(samples)
	, m_nplots(samples.size())
{}

NumberUpdateEvent::~NumberUpdateEvent() {}

const std::vector<float> NumberUpdateEvent::getSamples() const { return m_samples; }

#endif /* SPECTRUM_UPDATE_EVENTS_C */
