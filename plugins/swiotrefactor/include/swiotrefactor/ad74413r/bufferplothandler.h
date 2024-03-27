/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef BUFFERPLOTHANDLER_H
#define BUFFERPLOTHANDLER_H

#include "bufferlogic.h"
//#include "channelplotscalescontroller.h"

#include <QWidget>
#include <qmutex.h>
#include <QObject>

#include <deque>

namespace scopy::swiotrefactor {
#define DIAG_CHNLS_NUMBER 4
class BufferPlotHandler : public QWidget
{
	Q_OBJECT
public:
	BufferPlotHandler(QWidget *parent = nullptr, int plotChnlsNo = 0);
	~BufferPlotHandler();

	void setSingleCapture(bool en);
	void resetPlotParameters();
	void drawPlot();
	bool singleCapture() const;
	int getRequiredBuffersNumber();
public Q_SLOTS:
	void onBufferRefilled(QMap<int, std::vector<double>> data, int bufferCounter);
	void onTimespanChanged(double value);
	void onSamplingFrequencyComputed(double samplingFrequency);
Q_SIGNALS:
	void bufferDataReady(QMap<int, std::vector<double>> data);
	void singleCaptureFinished();

private:
	void resetDataPoints();
	void resetDeque();

	double m_plotSamplingFreq = 4800;
	double m_timespan = 1;
	int m_plotChnlsNo;

	// all of these will be calculated in functions; for example we will have a spinbox for timespan,
	// and in a slot we will set its value and we will calculate plotSampleRate and the number of necessary buffers
	int m_buffersNumber = 0;
	int m_bufferIndex = 0;
	int m_bufferSize = 0;
	int m_plotSampleRate = 0;

	bool m_singleCapture = false;

	QMap<int, std::deque<std::vector<double>>> m_dataPointsDeque;
	QMap<int, std::vector<double>> m_dataPoints;
	std::vector<bool> m_enabledPlots;
	QMutex *m_lock;
};
} // namespace scopy::swiotrefactor

#endif // BUFFERPLOTHANDLER_H
