/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef DATAPROCESSINGSERVICE_H
#define DATAPROCESSINGSERVICE_H

#include <QObject>
#include <QVector>
#include <plotmanager/dataprocessor.h>
#include <plotmanager/datamanager.h>

namespace scopy::extprocplugin {

class DataProcessingService : public QObject
{
	Q_OBJECT
public:
	explicit DataProcessingService(QObject *parent = nullptr);
	~DataProcessingService();

	void setSamplingFreq(double samplingFreq);
	void setFFTEnabled(bool enabled);

public Q_SLOTS:
	void processBufferData(QVector<QVector<float>> bufferData);

private:
	void computeXTime(int samplingFreq, int samples);
	void computeFFT(QVector<QVector<float>> bufferData);

	DataProcessor *m_dataProcessor;
	bool m_fftEnabled;
	double m_samplingFreq;
};

} // namespace scopy::extprocplugin

#endif // DATAPROCESSINGSERVICE_H