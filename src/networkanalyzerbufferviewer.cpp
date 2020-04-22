/*
 * Copyright 2019 Analog Devices, Inc.
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

#include "networkanalyzerbufferviewer.h"

#include "ui_networkanalyzerbufferviewer.h"

#include <qwt_plot_layout.h>

using namespace adiscope;

NetworkAnalyzerBufferViewer::NetworkAnalyzerBufferViewer(QWidget* parent)
	: QWidget(parent)
	, d_ui(new Ui::NetworkAnalyzerBufferViewer)
	, d_osc(nullptr)
	, d_selectedBuffersIndex(-1)
	, d_numBuffers(0)
{
	d_ui->setupUi(this);

	_setupPlot();
}

NetworkAnalyzerBufferViewer::~NetworkAnalyzerBufferViewer() { delete d_ui; }

void NetworkAnalyzerBufferViewer::setNumBuffers(unsigned int numBuffers) { d_numBuffers = numBuffers; }

void NetworkAnalyzerBufferViewer::pushBuffers(QPair<Buffer, Buffer> buffers)
{
	if (d_data.size() < d_numBuffers) {
		d_data.push_back(buffers);
	} else {
		static int currentBuffer = 0;
		d_data[currentBuffer++] = buffers;
		if (currentBuffer == d_numBuffers) {
			currentBuffer = 0;
		}
	}
}

void NetworkAnalyzerBufferViewer::selectBuffersAtIndex(int index, bool moveHandle)
{
	if (index != d_selectedBuffersIndex) {
		Q_EMIT indexChanged(index);
	}

	d_selectedBuffersIndex = index;

	static bool firstTime = true;
	if (firstTime) {
		firstTime = false;
	} else {
		d_plot->unregisterReferenceWaveform("data1");
		d_plot->unregisterReferenceWaveform("data2");
	}

	QVector<double> xData;

	double division = 1.0 / d_data[index].first.sampleRate;
	int n = d_data[index].first.buffer.size() / 2;

	for (int i = -n; i < n; ++i) {
		xData.push_back(division * i);
	}

	d_currentXdata.clear();
	d_currentXdata = xData;

	double max = 0.0;
	double min = 0.0;

	QVector<double> yData1, yData2;
	for (int i = 0; i < d_data[index].first.buffer.size(); ++i) {
		if (d_data[index].first.buffer[i] > max) {
			max = d_data[index].first.buffer[i];
		} else if (d_data[index].first.buffer[i] < min) {
			min = d_data[index].first.buffer[i];
		}
		yData1.push_back(d_data[index].first.buffer[i]);
	}
	for (int i = 0; i < d_data[index].second.buffer.size(); ++i) {
		if (d_data[index].second.buffer[i] > max) {
			max = d_data[index].second.buffer[i];
		} else if (d_data[index].second.buffer[i] < min) {
			min = d_data[index].second.buffer[i];
		}
		yData2.push_back(d_data[index].second.buffer[i]);
	}

	d_plot->setYaxis(min - 1.0, max + 1.0);
	d_plot->setXaxis(-n * division, n * division);
	d_plot->registerReferenceWaveform("data1", xData, yData1);
	d_plot->registerReferenceWaveform("data2", xData, yData2);

	// Move freq. handle on plot if prev/next button
	// are used to navigate through the buffers
	if (moveHandle) {
		Q_EMIT moveHandleAt(d_data[index].first.frequency);
	}
}

void NetworkAnalyzerBufferViewer::selectBuffers(double frequency)
{
	int index = -1;
	for (int i = 0; i < d_data.size() - 1; ++i) {
		if (d_data[i].first.frequency <= frequency && frequency <= d_data[i + 1].first.frequency) {
			index = i;
			break;
		}
		if (d_data[i + 1].first.frequency < frequency) {
			index = d_data.size() - 1;
		}
		if (frequency < d_data[i].first.frequency) {
			index = 0;
		}
	}

	if (index == -1) {
		return;
	}

	if (index >= d_data.size()) {
		return;
	}

	selectBuffersAtIndex(index, false);
}

void NetworkAnalyzerBufferViewer::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if (d_data.size() && d_selectedBuffersIndex == -1) {
		d_selectedBuffersIndex = 0;
		selectBuffersAtIndex(d_selectedBuffersIndex);
	}
}

void NetworkAnalyzerBufferViewer::setOscilloscope(Oscilloscope* osc) { d_osc = osc; }

void NetworkAnalyzerBufferViewer::clear()
{
	d_data.clear();
	d_selectedBuffersIndex = -1;
}

void NetworkAnalyzerBufferViewer::sendBufferToOscilloscope()
{
	if (d_selectedBuffersIndex < 0 || d_selectedBuffersIndex >= d_data.size()) {
		return;
	}

	d_osc->remove_ref_waveform("NA1");
	d_osc->remove_ref_waveform("NA2");

	QVector<double> yData1, yData2;
	auto index = d_selectedBuffersIndex;
	for (int i = 0; i < d_data[index].first.buffer.size(); ++i) {
		yData1.push_back(d_data[index].first.buffer[i]);
	}
	for (int i = 0; i < d_data[index].second.buffer.size(); ++i) {
		yData2.push_back(d_data[index].second.buffer[i]);
	}
	d_osc->add_ref_waveform("NA1", d_currentXdata, yData1, d_data[index].first.sampleRate);
	d_osc->add_ref_waveform("NA2", d_currentXdata, yData2, d_data[index].second.sampleRate);

	d_osc->detached();
}

void NetworkAnalyzerBufferViewer::btnPreviousClicked()
{
	if (d_selectedBuffersIndex - 1 >= 0) {
		selectBuffersAtIndex(d_selectedBuffersIndex - 1);
	}
}

void NetworkAnalyzerBufferViewer::btnNextClicked()
{
	if (d_selectedBuffersIndex + 1 < d_data.size()) {
		selectBuffersAtIndex(d_selectedBuffersIndex + 1);
	}
}

void NetworkAnalyzerBufferViewer::_setupPlot()
{
	d_plot = new TimeDomainDisplayPlot(d_ui->mainWidget);
	d_ui->mainLayout->addWidget(d_plot);

	d_plot->insertLegend(nullptr);
	d_plot->setUsingLeftAxisScales(false);
	int nrAxes = d_plot->axesCount(QwtPlot::yLeft);
	for (int i = 0; i < nrAxes; ++i) {
		d_plot->setAxisVisible(QwtAxisId(QwtPlot::yLeft, i), false);
	}
	d_plot->setAxisVisible(QwtAxisId(QwtPlot::xBottom, 0), false);

	d_plot->plotLayout()->setAlignCanvasToScales(true);
	d_ui->mainWidget->setMinimumHeight(150);
}
