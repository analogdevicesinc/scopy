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

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QVector>
#include <fftw3.h>

namespace scopy::extprocplugin {

enum class WindowType
{
	RECTANGULAR,
	HANN,
	BLACKMAN,
	HAMMING,
	KAISER
};

struct FFTResult
{
	QVector<float> frequency;
	QVector<float> magnitudeDB;
};

class DataProcessor : public QObject
{
	Q_OBJECT

public:
	explicit DataProcessor(QObject *parent = nullptr);
	~DataProcessor();

	FFTResult computeFFT(const QVector<float> &inputData, double samplingRate,
			     WindowType window = WindowType::RECTANGULAR);

	FFTResult computeComplexFFT(const QVector<float> &iData, const QVector<float> &qData, double samplingRate,
				    WindowType window = WindowType::RECTANGULAR);

	QVector<float> applyWindow(const QVector<float> &data, WindowType window);

	void setWindowType(WindowType window);
	WindowType getWindowType() const;

Q_SIGNALS:
	void fftCompleted(const FFTResult &result);

private:
	void initializeFFTW(int size);
	void cleanupFFTW();
	QVector<float> generateWindow(int size, WindowType window);
	QVector<float> computeMagnitude(const QVector<float> &real, const QVector<float> &imag);
	QVector<float> convertToDecibels(const QVector<float> &magnitude);
	QVector<float> generateFrequencyAxis(double samplingRate, int N);
	QVector<float> fftShift(const QVector<float> &data);

	WindowType m_windowType;
	int m_currentSize;
	fftwf_complex *m_fftwInput;
	fftwf_complex *m_fftwOutput;
	fftwf_plan m_forwardPlan;
	bool m_initialized;
};

} // namespace scopy::extprocplugin

#endif // DATAPROCESSOR_H
