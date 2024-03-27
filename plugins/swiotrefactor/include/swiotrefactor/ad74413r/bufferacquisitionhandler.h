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

#ifndef BUFFERACQUISITIONHANDLER_H
#define BUFFERACQUISITIONHANDLER_H

#include <QObject>
#include <qmutex.h>
#include <QMap>

namespace scopy::swiotrefactor {
#define DIAG_CHNLS_NUMBER 4
class BufferAcquisitionHandler : public QObject
{
	Q_OBJECT
public:
	BufferAcquisitionHandler(QObject *parent);
	~BufferAcquisitionHandler();

	void setSingleCapture(bool en);
	void resetPlotParameters();
	bool singleCapture() const;
	int getRequiredBuffersNumber();
public Q_SLOTS:
	void onBufferRefilled(QMap<int, QVector<double>> data, int bufferCounter);
	void onTimespanChanged(double value);
	void onSamplingFrequencyComputed(double samplingFrequency);
Q_SIGNALS:
	void bufferDataReady(QMap<int, QVector<double>> data);
	void singleCaptureFinished();

private:
	void resetDataPoints();

	double m_plotSamplingFreq = 4800;
	double m_timespan = 1;

	int m_buffersNumber = 0;
	int m_bufferIndex = 0;
	int m_bufferSize = 0;

	bool m_singleCapture = false;

	QMap<int, QVector<double>> m_dataPoints;
	QMutex *m_lock;
};
} // namespace scopy::swiotrefactor

#endif // BUFFERACQUISITIONHANDLER_H
