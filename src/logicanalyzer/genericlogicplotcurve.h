/*
 * Copyright (c) 2020 Analog Devices Inc.
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


#ifndef GENERICLOGICPLOTCURVE_H
#define GENERICLOGICPLOTCURVE_H

#include <qwt_plot_curve.h>

#include <QObject>

enum class LogicPlotCurveType : int {
	Data = 0,
	Annotations = 1,
};

class GenericLogicPlotCurve : public QObject, public QwtPlotCurve
{
	Q_OBJECT
public:
	GenericLogicPlotCurve(const QString &name = {}, const QString &id = {}, LogicPlotCurveType type = LogicPlotCurveType::Data,
			      double pixelOffset = 0.0, double traceHeight = 0.0, double sampleRate = 0.0,
			      double timeTriggerOffset = 0.0, uint64_t bufferSize = 0.0);

	QString getName() const;
	QString getId() const;
	double getPixelOffset() const;
	double getTraceHeight() const;
	double getSampleRate() const;
	double getTimeTriggerOffset() const;
	uint64_t getBufferSize() const;

	LogicPlotCurveType getType() const;

	void setName(const QString &name);
	void setId(const QString &id);
	void setPixelOffset(double pixelOffset);
	void setTraceHeight(double traceHeight);
	void setSampleRate(double sampleRate);
	void setTimeTriggerOffset(double timeTriggerOffset);
	void setBufferSize(uint64_t bufferSize);

	// Classes who inherit from GenericLogicPlotCurve must provide their
	// own behaviour for these methods
	virtual void dataAvailable(uint64_t from, uint64_t to) {}
	virtual void reset() {}

Q_SIGNALS:
	void nameChanged(QString);
	void pixelOffsetChanged(double);

protected:
	uint64_t fromTimeToSample(double time) const;
	double fromSampleToTime(uint64_t sample) const;

protected:
	QString m_name;
	QString m_id;
	double m_pixelOffset;
	double m_traceHeight;
	double m_sampleRate;
	double m_timeTriggerOffset;
	uint64_t m_bufferSize;
	LogicPlotCurveType m_type;
};

#endif // GENERICLOGICPLOTCURVE_H
