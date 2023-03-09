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

#ifndef NETWORKANALYZERBUFFERVIEWER_H
#define NETWORKANALYZERBUFFERVIEWER_H

#include <QWidget>
#include <QPushButton>
#include "oscilloscope.hpp"

#include "TimeDomainDisplayPlot.h"

namespace Ui {
class NetworkAnalyzerBufferViewer;
}

struct Buffer {
	Buffer() {}
	Buffer(double frequency, unsigned int sampleRate,
	       unsigned int bufferSize, const std::vector<float> &buffer):
		frequency(frequency), sampleRate(sampleRate),
		bufferSize(bufferSize), buffer(buffer) {}

	double frequency;
	unsigned int sampleRate;
	unsigned int bufferSize;
	std::vector<float> buffer;
};

namespace adiscope {
class NetworkAnalyzerBufferViewer : public QWidget
{
	Q_OBJECT

public:
	explicit NetworkAnalyzerBufferViewer(QWidget *parent = nullptr);
	~NetworkAnalyzerBufferViewer();

	void clear();
	void pushBuffers(QPair<Buffer, Buffer> buffers);
	void setOscilloscope(Oscilloscope *osc);

	QPair<Buffer, Buffer> getSelectedBuffers() const;

	void selectBuffersAtIndex(int index, bool moveHandle = true);
	void selectBuffers(double frequency);

	void setNumBuffers(unsigned int numBuffers);
Q_SIGNALS:
	void moveHandleAt(double);
	void indexChanged(int);

public Q_SLOTS:
	void setVisible(bool visible) Q_DECL_OVERRIDE;

	void sendBufferToOscilloscope();
	void btnPreviousClicked();
	void btnNextClicked();

private:
	void _setupPlot();

private:
	Ui::NetworkAnalyzerBufferViewer *d_ui;
	TimeDomainDisplayPlot *d_plot;
	QVector<QPair<Buffer, Buffer>> d_data;
	int d_selectedBuffersIndex;
	Oscilloscope *d_osc;
	QVector<double> d_currentXdata;
	int d_numBuffers;
};
}

#endif // NETWORKANALYZERBUFFERVIEWER_H
