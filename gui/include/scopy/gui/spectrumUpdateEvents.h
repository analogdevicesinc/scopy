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

#ifndef M2K_SPECTRUM_UPDATE_EVENTS_H
#define M2K_SPECTRUM_UPDATE_EVENTS_H

#include <gnuradio/high_res_timer.h>
#include <gnuradio/tags.h>

#include <QEvent>
#include <QString>

#include <complex>
#include <stdint.h>
#include <vector>

namespace scopy {
namespace gui {

static const int SpectrumUpdateEventType = 10005;
static const int SpectrumWindowCaptionEventType = 10008;
static const int SpectrumWindowResetEventType = 10009;
static const int SpectrumFrequencyRangeEventType = 10010;

class SpectrumUpdateEvent : public QEvent
{

public:
	SpectrumUpdateEvent(const float* fftPoints, const uint64_t numFFTDataPoints, const double* realTimeDomainPoints,
			    const double* imagTimeDomainPoints, const uint64_t numTimeDomainDataPoints,
			    const gr::high_res_timer_type dataTimestamp, const bool repeatDataFlag,
			    const bool lastOfMultipleUpdateFlag, const gr::high_res_timer_type generatedTimestamp,
			    const int droppedFFTFrames);

	~SpectrumUpdateEvent();

	const float* getFFTPoints() const;
	const double* getRealTimeDomainPoints() const;
	const double* getImagTimeDomainPoints() const;
	uint64_t getNumFFTDataPoints() const;
	uint64_t getNumTimeDomainDataPoints() const;
	gr::high_res_timer_type getDataTimestamp() const;
	bool getRepeatDataFlag() const;
	bool getLastOfMultipleUpdateFlag() const;
	gr::high_res_timer_type getEventGeneratedTimestamp() const;
	int getDroppedFFTFrames() const;

private:
	float* m_fftPoints;
	double* m_realDataTimeDomainPoints;
	double* m_imagDataTimeDomainPoints;
	uint64_t m_numFFTDataPoints;
	uint64_t m_numTimeDomainDataPoints;
	gr::high_res_timer_type m_dataTimestamp;
	bool m_repeatDataFlag;
	bool m_lastOfMultipleUpdateFlag;
	gr::high_res_timer_type m_eventGeneratedTimestamp;
	int m_droppedFFTFrames;
};

class SpectrumWindowCaptionEvent : public QEvent
{
public:
	SpectrumWindowCaptionEvent(const QString&);
	~SpectrumWindowCaptionEvent();
	QString getLabel();

private:
	QString m_labelString;
};

class SpectrumWindowResetEvent : public QEvent
{
public:
	SpectrumWindowResetEvent();
	~SpectrumWindowResetEvent();
};

class SpectrumFrequencyRangeEvent : public QEvent
{
public:
	SpectrumFrequencyRangeEvent(const double, const double, const double);
	~SpectrumFrequencyRangeEvent();
	double GetCenterFrequency() const;
	double GetStartFrequency() const;
	double GetStopFrequency() const;

private:
	double m_centerFrequency;
	double m_startFrequency;
	double m_stopFrequency;
};

class TimeUpdateEvent : public QEvent
{
public:
	TimeUpdateEvent(const std::vector<double*>& timeDomainPoints, const uint64_t numTimeDomainDataPoints,
			const std::vector<std::vector<gr::tag_t>>& tags);

	~TimeUpdateEvent();

	int which() const;
	const std::vector<double*> getTimeDomainPoints() const;
	uint64_t getNumTimeDomainDataPoints() const;
	bool getRepeatDataFlag() const;

	const std::vector<std::vector<gr::tag_t>> getTags() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_dataTimeDomainPoints;
	uint64_t m_numTimeDomainDataPoints;
	std::vector<std::vector<gr::tag_t>> m_tags;
};

/********************************************************************/

class IdentifiableTimeUpdateEvent : public TimeUpdateEvent
{
public:
	IdentifiableTimeUpdateEvent(const std::vector<double*>& timeDomainPoints,
				    const uint64_t numTimeDomainDataPoints,
				    const std::vector<std::vector<gr::tag_t>>& tags, const std::string& senderName);

	~IdentifiableTimeUpdateEvent();

	std::string senderName();

private:
	std::string m_senderName;
};

/********************************************************************/

class FreqUpdateEvent : public QEvent
{
public:
	FreqUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints);

	~FreqUpdateEvent();

	int which() const;
	const std::vector<double*> getPoints() const;
	uint64_t getNumDataPoints() const;
	bool getRepeatDataFlag() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_dataPoints;
	uint64_t m_numDataPoints;
};

class SetFreqEvent : public QEvent
{
public:
	SetFreqEvent(const double, const double);
	~SetFreqEvent();
	double getCenterFrequency() const;
	double getBandwidth() const;

private:
	double m_centerFrequency;
	double m_bandwidth;
};

/********************************************************************/

class ConstUpdateEvent : public QEvent
{
public:
	ConstUpdateEvent(const std::vector<double*>& realDataPoints, const std::vector<double*>& imagDataPoints,
			 const uint64_t numDataPoints);

	~ConstUpdateEvent();

	int which() const;
	const std::vector<double*> getRealPoints() const;
	const std::vector<double*> getImagPoints() const;
	uint64_t getNumDataPoints() const;
	bool getRepeatDataFlag() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_realDataPoints;
	std::vector<double*> m_imagDataPoints;
	uint64_t m_numDataPoints;
};

/********************************************************************/

class WaterfallUpdateEvent : public QEvent
{
public:
	WaterfallUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints,
			     const gr::high_res_timer_type dataTimestamp);

	~WaterfallUpdateEvent();

	int which() const;
	const std::vector<double*> getPoints() const;
	uint64_t getNumDataPoints() const;
	bool getRepeatDataFlag() const;

	gr::high_res_timer_type getDataTimestamp() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_dataPoints;
	uint64_t m_numDataPoints;

	gr::high_res_timer_type m_dataTimestamp;
};

/********************************************************************/

class TimeRasterUpdateEvent : public QEvent
{
public:
	TimeRasterUpdateEvent(const std::vector<double*>& dataPoints, const uint64_t numDataPoints);
	~TimeRasterUpdateEvent();

	int which() const;
	const std::vector<double*> getPoints() const;
	uint64_t getNumDataPoints() const;
	bool getRepeatDataFlag() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_dataPoints;
	uint64_t m_numDataPoints;
};

/********************************************************************/

class HistogramUpdateEvent : public QEvent
{
public:
	HistogramUpdateEvent(const std::vector<double*>& points, const uint64_t npoints);

	~HistogramUpdateEvent();

	int which() const;
	const std::vector<double*> getDataPoints() const;
	uint64_t getNumDataPoints() const;
	bool getRepeatDataFlag() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<double*> m_points;
	uint64_t m_npoints;
};

/********************************************************************/

class NumberUpdateEvent : public QEvent
{
public:
	NumberUpdateEvent(const std::vector<float>& samples);
	~NumberUpdateEvent();

	int which() const;
	const std::vector<float> getSamples() const;

	static QEvent::Type Type() { return QEvent::Type(SpectrumUpdateEventType); }

private:
	size_t m_nplots;
	std::vector<float> m_samples;
};
} // namespace gui
} // namespace scopy

#endif /* M2K_SPECTRUM_UPDATE_EVENTS_H */
