/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef PLOT_H
#define PLOT_H
#include "plotbuttonmanager.h"
#include "plotchannel.h"
#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QWidget>
#include <QwtPlot>
#include <QwtPlotZoomer>
#include <QSize>
#include <plotinfo.h>

#include <buffer_previewer.hpp>

namespace scopy {

class PlotScales;

class PlotAxis;
class PlotAxisHandle;
class PlotNavigator;
class PlotTracker;

typedef struct _PlotSamplingInfo
{
	uint32_t bufferSize = 0;
	uint32_t plotSize = 0;
	double sampleRate = 0;
	double startingPoint = 0;
	double freqOffset = 0;
	bool complexMode = 0;
	bool rollingMode = 0;
} SamplingInfo;

class SCOPY_GUI_EXPORT PlotWidget : public QWidget
{
	Q_OBJECT
public:
	PlotWidget(QWidget *parent = nullptr);
	~PlotWidget();

	void addPlotChannel(PlotChannel *ch);
	void removePlotChannel(PlotChannel *ch);
	QList<PlotChannel *> getChannels();

	QList<PlotAxis *> &plotAxis(int position);
	PlotAxis *xAxis();
	PlotAxis *yAxis();

	void addPlotAxis(PlotAxis *ax);
	// void removePlotAxis(PlotAxis *ax);  - not supported by Qwt

	bool eventFilter(QObject *object, QEvent *event) override;

	QwtPlot *plot() const;
	QGridLayout *layout();

	void addPlotAxisHandle(PlotAxisHandle *ax);
	void removePlotAxisHandle(PlotAxisHandle *ax);

	PlotChannel *selectedChannel() const;

	bool showXAxisLabels() const;
	void setShowXAxisLabels(bool newShowXAxisLabels);

	bool showYAxisLabels() const;
	void setShowYAxisLabels(bool newShowYAxisLabels);

	PlotInfo *getPlotInfo();

	PlotAxis *plotAxisFromId(QwtAxisId axisId);

	PlotNavigator *navigator() const;
	PlotTracker *tracker() const;
	PlotScales *scales() const;

	void setUnitsVisible(bool visible);

	void printPlot(QPainter *painter, bool useSymbols = false);
	void plotChannelChangeYAxis(PlotChannel *c, PlotAxis *y);
	void plotChannelChangeXAxis(PlotChannel *c, PlotAxis *x);

	PlotButtonManager *plotButtonManager() const;

public Q_SLOTS:
	void replot();
	void selectChannel(PlotChannel *);
	void showAxisLabels();
	void hideAxisLabels();
	void setAlignCanvasToScales(bool alignCanvasToScales);

Q_SIGNALS:
	void canvasSizeChanged();
	// These are only emitted if you set setMouseTracking(true) (not done by default)
	void mouseButtonPress(const QMouseEvent *event);
	void mouseButtonRelease(const QMouseEvent *event);
	void mouseMove(const QMouseEvent *event);
	void channelSelected(PlotChannel *ch);
	void addedChannel(PlotChannel *ch);
	void removedChannel(PlotChannel *ch);
	void plotScaleChanged();
	void newData();

protected:
	QSize minimumSizeHint() const override;

private:
	QwtPlot *m_plot;
	QwtPlotZoomer *m_zoomer;
	QGridLayout *m_layout;

	QList<PlotChannel *> m_plotChannels;

	QList<PlotAxis *> m_plotAxis[QwtAxis::AxisPositions];
	QList<PlotAxisHandle *> m_plotAxisHandles[QwtAxis::AxisPositions];

	PlotNavigator *m_navigator;
	PlotTracker *m_tracker;

	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;

	int m_xPosition;
	int m_yPosition;

	bool m_showXAxisLabels;
	bool m_showYAxisLabels;

	PlotChannel *m_selectedChannel;

	PlotInfo *m_plotInfo;
	PlotScales *m_plotScales;
	PlotButtonManager *m_plotButtonManager;

	void setupOpenGLCanvas();
	void setupNavigator();
	void setupPlotInfo();
	void setupPlotScales();
	void setupAxes();
	void setupPlotButtonManager();
	QwtSymbol::Style getCurveStyle(int i);
};

} // namespace scopy

#endif // PLOT_H
