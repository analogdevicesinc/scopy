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
#include "channelplotscalescontroller.h"
#include "linked_button.hpp"

#include <QWidget>
#include <qmutex.h>
#include <qobject.h>

#include <deque>
#include <gui/oscilloscope_plot.hpp>

namespace scopy::swiot {
#define DIAG_CHNLS_NUMBER 4
class BufferPlotHandler : public QWidget
{
	Q_OBJECT
public:
	explicit BufferPlotHandler(QWidget *parent = nullptr, int plotChnlsNo = 0);
	~BufferPlotHandler();

	QColor getCurveColor(int id) const;

	void setSingleCapture(bool en);
	void deleteResidualPlotData();
	void resetPlotParameters();
	void drawPlot();
	void setChnlsUnitOfMeasure(QVector<QString> unitsOfMeasure);
	void setChnlsRangeValues(QVector<std::pair<int, int>> rangeValues);
	void setHandlesName(QMap<int, QString> chnlsId);
	QWidget *getPlotWidget() const;
	bool singleCapture() const;
	int getRequiredBuffersNumber();
	void setUnitPerDivision(int channel, double unitPerDiv);
	void setInstantValue(int channel, double value);
	void addChannelScale(int index, QColor color, QString unit, bool enabled);
	void mapChannelCurveId(int curveId, int channelId);
	void setYAxisVisible(bool visible);
public Q_SLOTS:
	void onBufferRefilled(QVector<QVector<double>>, int bufferCounter);
	void onBtnExportClicked(QMap<int, bool> exportConfig);
	void onTimespanChanged(double value);
	void onSamplingFreqWritten(int samplingFreq);
	void onChannelWidgetEnabled(int curveId, bool en);
	void onChannelWidgetSelected(int curveId);
	void onPrintBtnClicked();
Q_SIGNALS:
	void singleCaptureFinished();
	void offsetHandleSelected(int hdlIdx, bool selected);
	void unitPerDivisionChanged(int i, double val);

private:
	void initPlot(int plotChnlsNo);
	void initStatusWidget();
	void resetDataPoints();
	void resetDeque();
	void readPreferences();
	void updatePlotTimespan();
	void updateScale(int channel);

	bool eventFilter(QObject *obj, QEvent *event);

	CapturePlot *m_plot;
	QWidget *m_plotWidget;
	QLabel *m_samplesAquiredLabel;
	QLabel *m_plotSamplesNumberLabel;
	LinkedButton *m_btnInfoStatus;
	ChannelPlotScalesController *m_plotScalesController;

	int m_samplingFreq = 4800;
	double m_timespan = 1;
	int m_plotChnlsNo;

	// all of these will be calculated in functions; for example we will have a spinbox for timespan,
	// and in a slot we will set its value and we will calculate plotSampleRate and the number of necessary buffers
	int m_buffersNumber = 0;
	int m_bufferIndex = 0;
	int m_bufferSize = 0;
	int m_plotSampleRate = 0;

	bool m_singleCapture = false;

	std::vector<std::deque<QVector<double>>> m_dataPointsDeque;
	std::vector<double *> m_dataPoints;
	std::vector<bool> m_enabledPlots;
	QMutex *m_lock;

	QMap<int, double> m_unitPerDivision;
	QMap<int, int> m_channelCurveId;
};
} // namespace scopy::swiot

#endif // BUFFERPLOTHANDLER_H
