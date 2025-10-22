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

#include "plotmanager/dataprocessor.h"
#include <QLoggingCategory>
#include <QtMath>
#include <algorithm>

Q_LOGGING_CATEGORY(CAT_DATA_PROCESSOR, "DataProcessor");

using namespace scopy::extprocplugin;

DataProcessor::DataProcessor(QObject *parent)
	: QObject(parent)
	, m_windowType(WindowType::RECTANGULAR)
	, m_currentSize(0)
	, m_fftwInput(nullptr)
	, m_fftwOutput(nullptr)
	, m_forwardPlan(nullptr)
	, m_initialized(false)
{}

DataProcessor::~DataProcessor() { cleanupFFTW(); }

FFTResult DataProcessor::computeFFT(const QVector<float> &inputData, double samplingRate, WindowType window)
{
	FFTResult result;

	if(inputData.isEmpty()) {
		qCWarning(CAT_DATA_PROCESSOR) << "Input data is empty";
		return result;
	}

	int N = inputData.size();

	if(N != m_currentSize || !m_initialized) {
		initializeFFTW(N);
	}

	QVector<float> windowedData = applyWindow(inputData, window);

	for(int i = 0; i < N; ++i) {
		m_fftwInput[i][0] = windowedData[i];
		m_fftwInput[i][1] = 0.0f;
	}

	fftwf_execute(m_forwardPlan);

	QVector<float> real(N);
	QVector<float> imaginary(N);

	for(int i = 0; i < N; ++i) {
		real[i] = m_fftwOutput[i][0];
		imaginary[i] = m_fftwOutput[i][1];
	}

	QVector<float> magnitude = computeMagnitude(real, imaginary);

	result.frequency = generateFrequencyAxis(samplingRate, N);
	result.magnitudeDB = convertToDecibels(fftShift(magnitude));

	Q_EMIT fftCompleted(result);

	return result;
}

FFTResult DataProcessor::computeComplexFFT(const QVector<float> &iData, const QVector<float> &qData,
					   double samplingRate, WindowType window)
{
	FFTResult result;

	if(iData.isEmpty() || qData.isEmpty() || iData.size() != qData.size()) {
		qCWarning(CAT_DATA_PROCESSOR) << "Invalid I/Q data for complex FFT";
		return result;
	}

	int N = iData.size();

	if(N != m_currentSize || !m_initialized) {
		initializeFFTW(N);
	}

	QVector<float> windowedI = applyWindow(iData, window);
	QVector<float> windowedQ = applyWindow(qData, window);

	for(int i = 0; i < N; ++i) {
		m_fftwInput[i][0] = windowedI[i];
		m_fftwInput[i][1] = windowedQ[i];
	}

	fftwf_execute(m_forwardPlan);

	QVector<float> real(N);
	QVector<float> imaginary(N);

	for(int i = 0; i < N; ++i) {
		real[i] = m_fftwOutput[i][0];
		imaginary[i] = m_fftwOutput[i][1];
	}

	QVector<float> magnitude = computeMagnitude(real, imaginary);

	result.frequency = generateFrequencyAxis(samplingRate, N);
	result.magnitudeDB = convertToDecibels(fftShift(magnitude));

	Q_EMIT fftCompleted(result);

	return result;
}

QVector<float> DataProcessor::applyWindow(const QVector<float> &data, WindowType window)
{
	QVector<float> windowedData = data;
	QVector<float> windowCoeffs = generateWindow(data.size(), window);

	for(int i = 0; i < data.size(); ++i) {
		windowedData[i] *= windowCoeffs[i];
	}

	return windowedData;
}

QVector<float> DataProcessor::computeMagnitude(const QVector<float> &real, const QVector<float> &imag)
{
	QVector<float> magnitude(real.size());

	for(int i = 0; i < real.size(); ++i) {
		magnitude[i] = sqrt(real[i] * real[i] + imag[i] * imag[i]);
	}

	return magnitude;
}

QVector<float> DataProcessor::convertToDecibels(const QVector<float> &magnitude)
{
	QVector<float> magnitudeDB(magnitude.size());

	for(int i = 0; i < magnitude.size(); ++i) {
		if(magnitude[i] > 0) {
			magnitudeDB[i] = 20.0f * log10(magnitude[i]);
		} else {
			magnitudeDB[i] = -120.0f;
		}
	}

	return magnitudeDB;
}

QVector<float> DataProcessor::generateFrequencyAxis(double samplingRate, int N)
{
	QVector<float> freq(N);
	double df = samplingRate / N;

	// Generate centered frequency axis (-fs/2 to fs/2) to avoid plotting discontinuity
	for(int i = 0; i < N; ++i) {
		freq[i] = (i - N / 2) * df;
	}

	return freq;
}

QVector<float> DataProcessor::fftShift(const QVector<float> &data)
{
	QVector<float> shifted(data.size());
	int N = data.size();
	int half = N / 2;

	// Move second half to first half, first half to second half
	for(int i = 0; i < half; ++i) {
		shifted[i] = data[i + half];
		shifted[i + half] = data[i];
	}

	return shifted;
}

void DataProcessor::setWindowType(WindowType window) { m_windowType = window; }

WindowType DataProcessor::getWindowType() const { return m_windowType; }

void DataProcessor::initializeFFTW(int size)
{
	cleanupFFTW();

	m_currentSize = size;
	m_fftwInput = fftwf_alloc_complex(size);
	m_fftwOutput = fftwf_alloc_complex(size);

	m_forwardPlan = fftwf_plan_dft_1d(size, m_fftwInput, m_fftwOutput, FFTW_FORWARD, FFTW_ESTIMATE);

	m_initialized = true;

	qCDebug(CAT_DATA_PROCESSOR) << "FFTW initialized for size:" << size;
}

void DataProcessor::cleanupFFTW()
{
	if(m_forwardPlan) {
		fftwf_destroy_plan(m_forwardPlan);
		m_forwardPlan = nullptr;
	}

	if(m_fftwInput) {
		fftwf_free(m_fftwInput);
		m_fftwInput = nullptr;
	}

	if(m_fftwOutput) {
		fftwf_free(m_fftwOutput);
		m_fftwOutput = nullptr;
	}

	m_initialized = false;
	m_currentSize = 0;
}

QVector<float> DataProcessor::generateWindow(int size, WindowType window)
{
	QVector<float> windowCoeffs(size);

	switch(window) {
	case WindowType::RECTANGULAR:
		std::fill(windowCoeffs.begin(), windowCoeffs.end(), 1.0f);
		break;

	case WindowType::HANN:
		for(int i = 0; i < size; ++i) {
			windowCoeffs[i] = 0.5f * (1.0f - cos(2.0f * M_PI * i / (size - 1)));
		}
		break;

	case WindowType::BLACKMAN:
		for(int i = 0; i < size; ++i) {
			windowCoeffs[i] = 0.42f - 0.5f * cos(2.0f * M_PI * i / (size - 1)) +
				0.08f * cos(4.0f * M_PI * i / (size - 1));
		}
		break;

	case WindowType::HAMMING:
		for(int i = 0; i < size; ++i) {
			windowCoeffs[i] = 0.54f - 0.46f * cos(2.0f * M_PI * i / (size - 1));
		}
		break;

	case WindowType::KAISER:
		for(int i = 0; i < size; ++i) {
			windowCoeffs[i] = 0.5f * (1.0f - cos(2.0f * M_PI * i / (size - 1)));
		}
		break;

	default:
		std::fill(windowCoeffs.begin(), windowCoeffs.end(), 1.0f);
		break;
	}

	return windowCoeffs;
}
