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

using namespace adiscope;

NetworkAnalyzerBufferViewer::NetworkAnalyzerBufferViewer(QWidget *parent) :
	QWidget(parent),
	d_ui(new Ui::NetworkAnalyzerBufferViewer),
	d_osc(nullptr),
	d_selectedBuffersIndex(-1)
{
	d_ui->setupUi(this);

	d_plot = new TimeDomainDisplayPlot(d_ui->mainWidget);
	d_ui->mainLayout->addWidget(d_plot);

	connect(d_ui->viewInOsc, &QPushButton::clicked,
		this, &NetworkAnalyzerBufferViewer::sendBufferToOscilloscope);
	connect(d_ui->previousBtn, &QPushButton::pressed,
		this, &NetworkAnalyzerBufferViewer::btnPreviousClicked);
	connect(d_ui->nextBtn, &QPushButton::pressed,
		this, &NetworkAnalyzerBufferViewer::btnNextClicked);

	d_plot->insertLegend(nullptr);

	d_ui->nextBtn->setDisabled(true);
	d_ui->previousBtn->setDisabled(true);
}

NetworkAnalyzerBufferViewer::~NetworkAnalyzerBufferViewer()
{
	delete d_ui;
}

void NetworkAnalyzerBufferViewer::pushBuffers(QPair<Buffer, Buffer> buffers)
{
	d_data.push_back(buffers);

	if (d_selectedBuffersIndex == -1) {
		d_selectedBuffersIndex = 0;
		selectBuffersAtIndex(d_selectedBuffersIndex);
		d_ui->nextBtn->setEnabled(true);
	}

	if (d_selectedBuffersIndex < d_data.size() - 1) {
		d_ui->nextBtn->setEnabled(true);
	}
}

void NetworkAnalyzerBufferViewer::selectBuffersAtIndex(int index)
{

	if (d_selectedBuffersIndex == index) {
		return;
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

	QVector<double> yData1, yData2;
	for (int i = 0; i < d_data[index].first.buffer.size(); ++i) {
		yData1.push_back(d_data[index].first.buffer[i]);
	}
	for (int i = 0; i < d_data[index].second.buffer.size(); ++i) {
		yData2.push_back(d_data[index].second.buffer[i]);
	}

	d_plot->setYaxis(-5, 5);
	d_plot->setXaxis(-n * division, n * division);
	d_plot->registerReferenceWaveform("data1", xData, yData1);
	d_plot->registerReferenceWaveform("data2", xData, yData2);

	// Move freq. handle on plot if prev/next button
	// are used to navigate through the buffers
	if (QObject::sender() == d_ui->previousBtn ||
			QObject::sender() == d_ui->nextBtn) {
		Q_EMIT moveHandleAt(d_data[index].first.frequency);
	}

	if (index == 0) {
		d_ui->previousBtn->setDisabled(true);
	} else if (index == d_data.size() - 1) {
		d_ui->nextBtn->setDisabled(true);
	} else {
		d_ui->previousBtn->setEnabled(true);
		d_ui->nextBtn->setEnabled(true);
	}
}

void NetworkAnalyzerBufferViewer::selectBuffers(double frequency)
{
	int index = -1;
	for (int i = 0; i < d_data.size() - 1; ++i) {
		if (d_data[i].first.frequency <= frequency
				&& frequency <= d_data[i + 1].first.frequency) {
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

	selectBuffersAtIndex(index);
}

void NetworkAnalyzerBufferViewer::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if (d_data.size()) {
		d_selectedBuffersIndex = 0;
		selectBuffersAtIndex(d_selectedBuffersIndex);
		d_ui->nextBtn->setEnabled(true);
	}
}

void NetworkAnalyzerBufferViewer::setOscilloscope(Oscilloscope *osc)
{
	d_osc = osc;
}

void NetworkAnalyzerBufferViewer::clear()
{
	d_data.clear();
	d_selectedBuffersIndex = -1;
	d_ui->previousBtn->setDisabled(true);
	d_ui->nextBtn->setDisabled(true);
}

void NetworkAnalyzerBufferViewer::sendBufferToOscilloscope()
{
	if (d_selectedBuffersIndex  < 0 || d_selectedBuffersIndex >= d_data.size()) {
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

	d_ui->nextBtn->setEnabled(true);

	if (d_selectedBuffersIndex == 0) {
		d_ui->previousBtn->setDisabled(true);
	}
}

void NetworkAnalyzerBufferViewer::btnNextClicked()
{
	if (d_selectedBuffersIndex + 1 < d_data.size()) {
		selectBuffersAtIndex(d_selectedBuffersIndex + 1);
	}

	d_ui->previousBtn->setEnabled(true);

	if (d_selectedBuffersIndex == d_data.size() - 1) {
		d_ui->nextBtn->setDisabled(true);
	}
}
