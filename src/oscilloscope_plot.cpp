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

#include "oscilloscope_plot.hpp"
#include "symbol_controller.h"
#include "handles_area.hpp"
#include "plot_line_handle.h"

#include "logicanalyzer/logicdatacurve.h"
#include "logicanalyzer/annotationcurve.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QThread>
#include <QDebug>

#include <algorithm>

#define ERROR_VALUE -10000000

using namespace adiscope;

/*
 * OscilloscopePlot class
 */
OscilloscopePlot::OscilloscopePlot(QWidget *parent,
			unsigned int xNumDivs, unsigned int yNumDivs):
	TimeDomainDisplayPlot(parent, xNumDivs, yNumDivs)
{
	setYaxisUnit("V");

	setMinXaxisDivision(100E-9); // A minimum division of 100 nano second
	setMaxXaxisDivision(1E-3); // A maximum division of 1 milli second - until adding decimation
	setMinYaxisDivision(1E-6); // A minimum division of 1 micro Volts
	setMaxYaxisDivision(10.0); // A maximum division of 10 Volts
}

OscilloscopePlot::~OscilloscopePlot()
{
}

/*
 * CapturePlot class
 */
CapturePlot::CapturePlot(QWidget *parent,
			 unsigned int xNumDivs, unsigned int yNumDivs):
	OscilloscopePlot(parent, xNumDivs, yNumDivs),
	d_triggerAEnabled(false),
	d_triggerBEnabled(false),
	d_selected_channel(-1),
	d_measurementsEnabled(false),
	d_cursorReadoutsVisible(false),
	d_bufferSizeLabelVal(0),
	d_sampleRateLabelVal(1.0),
	d_labelsEnabled(false),
	d_timeTriggerMinValue(-1),
	d_timeTriggerMaxValue(1),
	d_trackMode(false),
	horizCursorsLocked(false),
	vertCursorsLocked(false),
	d_horizCursorsEnabled(false),
	d_vertCursorsEnabled(false),
	d_bonusWidth(0),
	d_gatingEnabled(false),
	m_conversion_function(nullptr),
	d_startedGrouping(false),
	d_bottomHandlesArea(nullptr),
	d_xAxisInterval{0.0, 0.0},
	d_currentHandleInitPx(30),
	d_maxBufferError(nullptr)
{
	setMinimumHeight(250);
	setMinimumWidth(500);

	/* Initial colors scheme */
	d_trigAactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	d_trigAinactiveLinePen = QPen(QColor(175, 175, 175, 150), 2, Qt::DashLine);
	d_trigBactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	d_trigBinactiveLinePen = QPen(QColor(175, 175, 175), 2, Qt::DashLine);
	d_timeTriggerInactiveLinePen = QPen(QColor(74, 100, 255, 150), 2, Qt::DashLine);
	d_timeTriggerActiveLinePen = QPen(QColor(74, 100, 255), 2, Qt::SolidLine);
	/* End of: Initial colors scheme */

	markerIntersection1 = new QwtPlotMarker();
	markerIntersection2 = new QwtPlotMarker();
	markerIntersection1->setSymbol(new QwtSymbol(
			QwtSymbol::Ellipse, QColor(237, 28, 36),
			QPen(QColor(255, 255 ,255, 140), 2, Qt::SolidLine),
			QSize(5, 5)));
	markerIntersection2->setSymbol(new QwtSymbol(
		       QwtSymbol::Ellipse, QColor(237, 28, 36),
		       QPen(QColor(255, 255 ,255, 140), 2, Qt::SolidLine),
		       QSize(5, 5)));

	d_symbolCtrl = new SymbolController(this);

	setHorizUnitsPerDiv(1E-6);
	zoomBaseUpdate();

	/* Adjacent areas (top/bottom/left/right) */
	d_topWidget = new QWidget(this);
	d_topHandlesArea = new GateHandlesArea(this->canvas());
	d_bottomHandlesArea = new HorizHandlesArea(this->canvas());
	d_leftHandlesArea = new VertHandlesArea(this->canvas());
	d_rightHandlesArea = new VertHandlesArea(this->canvas());

	d_topWidget->setStyleSheet("QWidget {background-color: transparent}");
	d_topWidget->setMinimumHeight(50);
	d_topHandlesArea->setMinimumHeight(20);
	d_topHandlesArea->setLargestChildWidth(80);
	d_bottomHandlesArea->setMinimumHeight(50);
	d_leftHandlesArea->setMinimumWidth(50);
	d_rightHandlesArea->setMinimumWidth(50);
	d_bottomHandlesArea->setLargestChildWidth(60);
	d_rightHandlesArea->setLargestChildHeight(60);
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());
	d_rightHandlesArea->setMinimumHeight(this->minimumHeight());

	d_topHandlesArea->hide();
	/* Add content to the top area of the plot */
	// Time Base
	d_timeBaseLabel = new QLabel(this);
	d_timeBaseLabel->setStyleSheet("QLabel {"
		"color: #4a64ff;"
		"font-weight: bold;"
		"}");

	// Call to minimumSizeHint() is required. Otherwise font properties from
	// stylesheet will be ignored when calculating width using FontMetrics
	int width = d_timeBaseLabel->minimumSizeHint().width();
	QFontMetrics fm = d_timeBaseLabel->fontMetrics();
	width = fm.width("999.999 ms/div");
	d_timeBaseLabel->setMinimumWidth(width);

	// Sample Rate and Buffer Size
	d_sampleRateLabel = new QLabel("", this);

	// Trigger State
	d_triggerStateLabel = new QLabel(this);

	d_maxBufferError = new QLabel(this);
	d_maxBufferError->setStyleSheet("QLabel {"
		"color: #ff0000;"
		"}");

	d_maxBufferError->setWordWrap(true);

	// Top area layout
	QHBoxLayout *topWidgetLayout = new QHBoxLayout(d_topWidget);
	topWidgetLayout->setContentsMargins(d_leftHandlesArea->minimumWidth(),
		0, d_rightHandlesArea->minimumWidth(), 5);

	topWidgetLayout->setSpacing(10);

	topWidgetLayout->insertWidget(0, d_timeBaseLabel, 0, Qt::AlignLeft |
		Qt::AlignBottom);
	topWidgetLayout->insertWidget(1, d_sampleRateLabel, 0, Qt::AlignLeft |
		Qt::AlignBottom);
	topWidgetLayout->insertWidget(2, d_maxBufferError, 0, Qt::AlignRight |
		Qt::AlignBottom);
	topWidgetLayout->insertWidget(3, d_triggerStateLabel, 0, Qt::AlignRight |
		Qt::AlignBottom);

	QSpacerItem *spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding,
		QSizePolicy::Fixed);
	topWidgetLayout->insertSpacerItem(2, spacerItem);

	d_topWidget->setLayout(topWidgetLayout);


	/* Time trigger widget */
	d_timeTriggerBar = new VertBar(this);
	d_symbolCtrl->attachSymbol(d_timeTriggerBar);
	d_timeTriggerBar->setPen(d_timeTriggerInactiveLinePen);
	d_timeTriggerBar->setCanLeavePlot(true);

	d_timeTriggerHandle = new FreePlotLineHandleH(
					QPixmap(":/icons/time_trigger_handle.svg"),
					QPixmap(":/icons/time_trigger_left.svg"),
					QPixmap(":/icons/time_trigger_right.svg"),
					d_bottomHandlesArea);
	d_timeTriggerHandle->setPen(d_timeTriggerActiveLinePen);

	connect(d_timeTriggerHandle, SIGNAL(grabbedChanged(bool)),
		SLOT(onTimeTriggerHandleGrabbed(bool)));

	connect(d_timeTriggerHandle, SIGNAL(positionChanged(int)),
		SLOT(onTimeTriggerHandlePosChanged(int)));

	/* When bar position changes due to plot resizes update the handle */
	connect(d_timeTriggerBar, &VertBar::pixelPositionChanged,
		[=](int pos) {
				updateHandleAreaPadding(d_labelsEnabled);
				d_timeTriggerHandle->setPositionSilenty(pos);
		});

	connect(d_timeTriggerHandle, &FreePlotLineHandleH::positionChanged,
		d_timeTriggerBar, &VertBar::setPixelPosition);

	connect(d_timeTriggerHandle, &RoundedHandleV::mouseReleased,
		[=]() {
			double pos = d_timeTriggerHandle->position();

			QwtScaleMap xMap = this->canvasMap(QwtAxisId(QwtPlot::xBottom, 0));
			double min = -(xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();
			double max = (xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();

			xMap.setScaleInterval(min, max);
			double time = xMap.invTransform(pos);

			if (time < d_timeTriggerMinValue) {
				d_timeTriggerBar->setPixelPosition(
							xMap.transform(d_timeTriggerMinValue));
			}
			if (time > d_timeTriggerMaxValue) {
				d_timeTriggerBar->setPixelPosition(
							xMap.transform(d_timeTriggerMaxValue));
			}
	});

	/* Level triggers widgets */
	// Trigger A
	d_levelTriggerABar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(d_levelTriggerABar);
	d_levelTriggerABar->setPen(d_timeTriggerInactiveLinePen);
	d_levelTriggerABar->setCanLeavePlot(true);

	d_levelTriggerAHandle = new FreePlotLineHandleV(
					QPixmap(":/icons/level_trigger_handle.svg"),
					QPixmap(":/icons/level_trigger_up.svg"),
					QPixmap(":/icons/level_trigger_down.svg"),
					d_rightHandlesArea);
	d_levelTriggerAHandle->setPen(d_timeTriggerActiveLinePen);

	d_levelTriggerABar->setVisible(false);
	d_levelTriggerAHandle->setVisible(false);

	/* When a handle position changes the bar follows */
	connect(d_levelTriggerAHandle, SIGNAL(positionChanged(int)),
		d_levelTriggerABar, SLOT(setPixelPosition(int)));
	/* When bar position changes due to plot resizes update the handle */
	connect(d_levelTriggerABar, &HorizBar::pixelPositionChanged,
		[=](int pos) {
			d_levelTriggerAHandle->setPositionSilenty(pos);
		});
	/* When handle is grabbed change bar drawing style */
	connect(d_levelTriggerAHandle, SIGNAL(grabbedChanged(bool)),
		SLOT(onTriggerAHandleGrabbed(bool)));

	// Trigger B
	d_levelTriggerBBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(d_levelTriggerBBar);
	d_levelTriggerBBar->setPen(d_trigBinactiveLinePen);
	d_levelTriggerBBar->setCanLeavePlot(true);

	d_levelTriggerBHandle = new FreePlotLineHandleV(
					QPixmap(":/icons/level_trigger_handle.svg"),
					QPixmap(":/icons/level_trigger_up.svg"),
					QPixmap(":/icons/level_trigger_down.svg"),
					d_rightHandlesArea);
	d_levelTriggerBHandle->setPen(d_trigBactiveLinePen);

	d_levelTriggerBBar->setVisible(false);
	d_levelTriggerBHandle->setVisible(false);

	/* When a handle position changes the bar follows */
	connect(d_levelTriggerBHandle, SIGNAL(positionChanged(int)),
		d_levelTriggerBBar, SLOT(setPixelPosition(int)));
	/* When bar position changes due to plot resizes update the handle */
	connect(d_levelTriggerBBar, &HorizBar::pixelPositionChanged,
		[=](int pos) {
			d_levelTriggerBHandle->setPositionSilenty(pos);
		});
	/* When handle is grabbed change bar drawing style */
	connect(d_levelTriggerBHandle, SIGNAL(grabbedChanged(bool)),
		SLOT(onTriggerBHandleGrabbed(bool)));


	/* Measurement Cursors */
	d_vCursorHandle1 = new PlotLineHandleV(
				QPixmap(":/icons/v_cursor_handle.svg"),
				d_rightHandlesArea);
	d_vCursorHandle2 = new PlotLineHandleV(
				QPixmap(":/icons/v_cursor_handle.svg"),
				d_rightHandlesArea);
	d_hCursorHandle1 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);
	d_hCursorHandle2 = new PlotLineHandleH(
				QPixmap(":/icons/h_cursor_handle.svg"),
				d_bottomHandlesArea);

	/* Measurement gate cursors */
	d_hGatingHandle1 = new PlotGateHandle(
				QPixmap(":/icons/gate_handle.svg"),
				d_topHandlesArea);

	d_hGatingHandle2 = new PlotGateHandle(
				QPixmap(":/icons/gate_handle.svg"),
				d_topHandlesArea);

	d_hGatingHandle1->setCenterLeft(false);
	d_vBar1 = new VertBar(this, true);
	d_vBar2 = new VertBar(this, true);
	d_hBar1 = new HorizBar(this, true);
	d_hBar2 = new HorizBar(this, true);

	d_gateBar1 = new VertBar(this,true);
	d_gateBar2 = new VertBar(this,true);

	d_gateBar1->setVisible(false);
	d_gateBar2->setVisible(false);

	d_symbolCtrl->attachSymbol(d_vBar1);
	d_symbolCtrl->attachSymbol(d_vBar2);
	d_symbolCtrl->attachSymbol(d_hBar1);
	d_symbolCtrl->attachSymbol(d_hBar2);

	d_symbolCtrl->attachSymbol(d_gateBar1);
	d_symbolCtrl->attachSymbol(d_gateBar2);

	QPen cursorsLinePen = QPen(QColor(155, 155, 155), 1, Qt::DashLine);
	d_hBar1->setPen(cursorsLinePen);
	d_hBar2->setPen(cursorsLinePen);
	d_vBar1->setPen(cursorsLinePen);
	d_vBar2->setPen(cursorsLinePen);

	d_vCursorHandle1->setPen(cursorsLinePen);
	d_vCursorHandle2->setPen(cursorsLinePen);
	d_hCursorHandle1->setPen(cursorsLinePen);
	d_hCursorHandle2->setPen(cursorsLinePen);


	/* gate bars */
	QPen gatePen = QPen(QColor(255,255,255),1,Qt::SolidLine);
	d_gateBar1->setPen(gatePen);
	d_gateBar2->setPen(gatePen);

	d_gateBar1->setVisible(false);
	d_gateBar2->setVisible(false);

	d_hGatingHandle1->hide();
	d_hGatingHandle2->hide();

	d_vCursorHandle1->hide();
	d_vCursorHandle2->hide();
	d_hCursorHandle1->hide();
	d_hCursorHandle2->hide();

	d_vBar1->setVisible(false);
	d_vBar2->setVisible(false);
	d_hBar1->setVisible(false);
	d_hBar2->setVisible(false);

	d_cursorReadouts = new CursorReadouts(this);
	d_cursorReadouts->setTopLeftStartingPoint(QPoint(8, 8));
	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	/* Set initial values for cursors */
	double voltsPerDiv = VertUnitsPerDiv(activeVertAxis());
	double secPerDiv = HorizUnitsPerDiv();

	/* Update Cursor Readouts */
	onVoltageCursor1Moved(d_hBar1->plotCoord().y());
	onVoltageCursor2Moved(d_hBar2->plotCoord().y());
	onTimeCursor1Moved(d_vBar1->plotCoord().x());
	onTimeCursor2Moved(d_vBar2->plotCoord().x());

	d_cursorMetricFormatter.setTwoDecimalMode(false);
	d_cursorTimeFormatter.setTwoDecimalMode(false);

	/* When a handle position changes the bar follows */
	connect(d_vCursorHandle1, &PlotLineHandleV::positionChanged,
		[=](int value) {

		if (vertCursorsLocked) {
			int position2 = value - (pixelPosHandleVert1 - pixelPosHandleVert2);
			pixelPosHandleVert2 = position2;
			d_hBar2->setPixelPosition(position2);
		}

		pixelPosHandleVert1 = value;
		d_hBar1->setPixelPosition(value);
	});
	connect(d_vCursorHandle2, &PlotLineHandleV::positionChanged,
		[=](int value) {

		if (vertCursorsLocked) {
			int position1 = value + (pixelPosHandleVert1 - pixelPosHandleVert2);
			pixelPosHandleVert1 = position1;
			d_hBar1->setPixelPosition(position1);
		}

		pixelPosHandleVert2 = value;
		d_hBar2->setPixelPosition(value);
	});

	connect(d_hCursorHandle1, &PlotLineHandleH::positionChanged,
		[=](int value) {
		if (horizCursorsLocked) {
			int position2 = value - (pixelPosHandleHoriz1 - pixelPosHandleHoriz2);
			pixelPosHandleHoriz2 = position2;
			d_vBar2->setPixelPosition(position2);
		}
		pixelPosHandleHoriz1 = value;
		d_vBar1->setPixelPosition(value);
	});
	connect(d_hCursorHandle2, &PlotLineHandleH::positionChanged,
		[=](int value) {
		if (horizCursorsLocked) {
			int position1 = value + (pixelPosHandleHoriz1 - pixelPosHandleHoriz2);
			pixelPosHandleHoriz1 = position1;
			d_vBar1->setPixelPosition(position1);
		}
		pixelPosHandleHoriz2 = value;
		d_vBar2->setPixelPosition(value);
	});

	connect(d_hGatingHandle1, &PlotLineHandleH::positionChanged,[=](int value){
		d_hGatingHandle2->setOtherCursorPosition(d_hGatingHandle1->position());
		/* make sure that the gate handles don't cross each other */
		if(d_hGatingHandle1->position() <= d_hGatingHandle2->position()){

			d_gateBar1->setPixelPosition(value);

		}
		else{
			d_gateBar1->setPixelPosition(d_hGatingHandle2->position());
			d_hGatingHandle1->setPosition(d_hGatingHandle2->position());
		}
	});

	connect(d_hGatingHandle2, &PlotLineHandleH::positionChanged,[=](int value){
		d_hGatingHandle1->setOtherCursorPosition(d_hGatingHandle2->position());
		/* make sure that the gate handles don't cross each other */
		if(d_hGatingHandle2->position() >= d_hGatingHandle1->position()){
			d_gateBar2->setPixelPosition(value);
		}
		else{
			d_gateBar2->setPixelPosition(d_hGatingHandle1->position());
			d_hGatingHandle2->setPosition(d_hGatingHandle1->position());
		}
	});

	d_hBar1->setPosition(0 + voltsPerDiv);
	d_hBar2->setPosition(0 - voltsPerDiv);
	d_vBar1->setPosition(0 + secPerDiv);
	d_vBar2->setPosition(0 - secPerDiv);

	d_gateBar1->setPosition(0 - 4 * secPerDiv);
	d_gateBar2->setPosition(0 + 4 * secPerDiv);

	/* initialise gate handle positions */
	d_hGatingHandle1->setOtherCursorPosition(d_hGatingHandle2->position());
	d_hGatingHandle2->setOtherCursorPosition(d_hGatingHandle1->position());

	/* When bar position changes due to plot resizes update the handle */
	connect(d_hBar1, SIGNAL(pixelPositionChanged(int)),
			SLOT(onHbar1PixelPosChanged(int)));
	connect(d_hBar2, SIGNAL(pixelPositionChanged(int)),
			SLOT(onHbar2PixelPosChanged(int)));
	connect(d_vBar1, SIGNAL(pixelPositionChanged(int)),
			SLOT(onVbar1PixelPosChanged(int)));
	connect(d_vBar2, SIGNAL(pixelPositionChanged(int)),
			SLOT(onVbar2PixelPosChanged(int)));

	connect(d_vBar1, SIGNAL(positionChanged(double)),
			SLOT(onTimeCursor1Moved(double)));
	connect(d_vBar2, SIGNAL(positionChanged(double)),
			SLOT(onTimeCursor2Moved(double)));
	connect(d_hBar1, SIGNAL(positionChanged(double)),
			SLOT(onVoltageCursor1Moved(double)));
	connect(d_hBar2, SIGNAL(positionChanged(double)),
			SLOT(onVoltageCursor2Moved(double)));

	connect(d_timeTriggerHandle, &FreePlotLineHandleH::reset, [=](){
		Q_EMIT timeTriggerValueChanged(0);
	});
	connect(d_levelTriggerAHandle, &FreePlotLineHandleV::reset, [=](){
		d_levelTriggerABar->setPlotCoord(
					QPointF(d_levelTriggerABar->plotCoord().x(), 0));
	});

	connect(d_gateBar1,SIGNAL(pixelPositionChanged(int)),
			SLOT(onGateBar1PixelPosChanged(int)));
	connect(d_gateBar2,SIGNAL(pixelPositionChanged(int)),
			SLOT(onGateBar2PixelPosChanged(int)));

	connect(d_gateBar1,SIGNAL(positionChanged(double)),
			SLOT(onGateBar1Moved(double)));
	connect(d_gateBar2,SIGNAL(positionChanged(double)),
			SLOT(onGateBar2Moved(double)));


	/* Apply measurements for every new batch of data */
	connect(this, SIGNAL(newData()),
		SLOT(onNewDataReceived()));

	/* Add offset widgets for each new channel */
	connect(this, SIGNAL(channelAdded(int)),
		SLOT(onChannelAdded(int)));

	connect(this, &TimeDomainDisplayPlot::digitalPlotCurveAdded,
		this, &CapturePlot::onDigitalChannelAdded);

	installEventFilter(this);
	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
    const int fmw = QFontMetrics(scaleWidget->font()).width("-XXX.XXX XX");
    scaleWidget->setMinBorderDist(fmw / 2 + 30, fmw / 2 + 30);

	displayGraticule = false;

	graticule = new Graticule(this);
	connect(this, SIGNAL(canvasSizeChanged()),graticule,SLOT(onCanvasSizeChanged()));

	QBrush gateBrush = QBrush(QColor(0,30,150,90));
	gateBrush.setStyle(Qt::SolidPattern);

	/* configure the measurement gates */
	leftGate = new QwtPlotShapeItem();
	leftGate->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
	leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	leftGateRect.setLeft(axisScaleDiv(xBottom).lowerBound());
	leftGateRect.setRight(d_gateBar1->plotCoord().x());
	leftGate->setRect(leftGateRect);
	leftGate->setBrush(gateBrush);

	rightGate = new QwtPlotShapeItem();
	rightGate->setAxes(QwtPlot::xBottom,QwtPlot::yRight);
	rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	rightGateRect.setLeft(d_gateBar2->plotCoord().x());
	rightGateRect.setRight(axisScaleDiv(xBottom).upperBound());
	rightGate->setRect(rightGateRect);
	rightGate->setBrush(gateBrush);
}

CapturePlot::~CapturePlot()
{
	markerIntersection1->detach();
	markerIntersection2->detach();
	removeEventFilter(this);
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
	delete markerIntersection1;
	delete markerIntersection2;
	for (auto it = d_measureObjs.begin(); it != d_measureObjs.end(); ++it) {
		delete *it;
	}
	delete graticule;
	delete leftGate;
	delete rightGate;
}

void CapturePlot::replot()
{

	TimeDomainDisplayPlot::replot();

	if (!d_bottomHandlesArea) {
		return;
	}

	const QwtInterval interval = axisInterval(QwtPlot::xBottom);
	if (interval.minValue() != d_xAxisInterval.first
			|| interval.maxValue() != d_xAxisInterval.second) {

		d_bottomHandlesArea->repaint();

		d_xAxisInterval.first = interval.minValue();
		d_xAxisInterval.second = interval.maxValue();
	}
}

HorizBar *CapturePlot::levelTriggerA()
{
	return d_levelTriggerABar;
}

HorizBar *CapturePlot::levelTriggerB()
{
	return d_levelTriggerBBar;
}

void CapturePlot::enableTimeTrigger(bool enable)
{
	d_timeTriggerBar->setVisible(enable);
	d_timeTriggerHandle->setVisible(enable);
}

void CapturePlot::onHbar1PixelPosChanged(int pos)
{
	d_vCursorHandle1->setPositionSilenty(pos);
}

void CapturePlot::onHbar2PixelPosChanged(int pos)
{
	d_vCursorHandle2->setPositionSilenty(pos);
}

void CapturePlot::onVbar1PixelPosChanged(int pos)
{
	d_hCursorHandle1->setPositionSilenty(pos);
	displayIntersection();
}

void CapturePlot::onVbar2PixelPosChanged(int pos)
{
	displayIntersection();
	d_hCursorHandle2->setPositionSilenty(pos);
}

void CapturePlot::onTimeCursor1Moved(double value)
{
	QString text;

	text = d_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor1Text(text);
	d_cursorReadoutsText.t1 = text;

	double diff = value - d_vBar2->plotCoord().x();
	text = d_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	if (diff !=0 )
		text = d_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
	d_cursorReadoutsText.freq = text;

	if (d_trackMode) {
		onVoltageCursor1Moved(getHorizontalCursorIntersection(d_vBar1->plotCoord().x()));
	}

	value_v1 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onTimeCursor2Moved(double value)
{
	QString text;

	text = d_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor2Text(text);
	d_cursorReadoutsText.t2 = text;

	double diff = d_vBar1->plotCoord().x() - value;
	text = d_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	if (diff !=0 )
		text = d_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
	d_cursorReadoutsText.freq = text;

	if (d_trackMode) {
		onVoltageCursor2Moved(getHorizontalCursorIntersection(d_vBar2->plotCoord().x()));
	}

	value_v2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onVoltageCursor1Moved(double value)
{
	QString text;

	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor1Text(error ? "-" : text);
	d_cursorReadoutsText.v1 = error ? "-" : text;

	double valueCursor2;
	if (d_trackMode) {
		valueCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());
	} else {
		valueCursor2 = d_hBar2->plotCoord().y();
	}

	double diff = value - (valueCursor2 * d_displayScale) ;
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	value_h1 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onVoltageCursor2Moved(double value)
{
	QString text;

	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor2Text(error ? "-" : text);
	d_cursorReadoutsText.v2 = error ? "-" : text;

	double valueCursor1;
	if (d_trackMode) {
		valueCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	} else {
		valueCursor1 = d_hBar1->plotCoord().y();
	}

	double diff = (valueCursor1 * d_displayScale) - value;
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	value_h2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onGateBar1PixelPosChanged(int pos)
{
	d_hGatingHandle1->setPositionSilenty(pos);
	d_hGatingHandle2->setOtherCursorPosition(d_hGatingHandle1->position());
}

void CapturePlot::onGateBar2PixelPosChanged(int pos)
{
	d_hGatingHandle2->setPositionSilenty(pos);
	d_hGatingHandle1->setOtherCursorPosition(d_hGatingHandle2->position());
}

void CapturePlot::onGateBar1Moved(double value)
{

	if (d_selected_channel < 0) {
		return;
	}

	//update gate handle
	leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	leftGateRect.setLeft(axisScaleDiv(xBottom).lowerBound());
	leftGateRect.setRight(value);
	leftGate->setRect(leftGateRect);

	int n = Curve(d_selected_channel)->data()->size();

	double maxTime = 0;
	double minTime = 0;

	if (n == 0) {
		maxTime = axisScaleDiv(xBottom).upperBound();
		minTime = axisScaleDiv(xBottom).lowerBound();
	} else {
		maxTime = Curve(d_selected_channel)->data()->sample(n-1).x();
		minTime = Curve(d_selected_channel)->data()->sample(0).x();
	}

	//data index to start measurement
	int currentIndex = (value - minTime) / (maxTime-minTime) * n;

	for (int i = 0; i < d_measureObjs.size(); i++) {
		Measure *measure = d_measureObjs[i];
		measure->setStartIndex(currentIndex);
	}

	value_gateLeft = value;
	//find the percentage of the gate in relation with plot width
	double width = (value - axisScaleDiv(xBottom).lowerBound()) / (axisScaleDiv(xBottom).upperBound() - axisScaleDiv(xBottom).lowerBound());
	Q_EMIT leftGateChanged(width);
	d_hGatingHandle1->setTimeValue(d_gateBar1->plotCoord().x());

	replot();
}

void CapturePlot::onGateBar2Moved(double value)
{

	if (d_selected_channel < 0) {
		return;
	}

	//update gate handle
	rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	rightGateRect.setLeft(value);
	rightGateRect.setRight(axisScaleDiv(xBottom).upperBound());
	rightGate->setRect(rightGateRect);

	int n = Curve(d_selected_channel)->data()->size();

	double maxTime = 0;
	double minTime = 0;

	if (n == 0) {
		maxTime = axisScaleDiv(xBottom).upperBound();
		minTime = axisScaleDiv(xBottom).lowerBound();
	} else {
		maxTime = Curve(d_selected_channel)->data()->sample(n-1).x();
		minTime = Curve(d_selected_channel)->data()->sample(0).x();
	}

	//data index to end measurement
	int currentIndex = (value - minTime) / (maxTime-minTime) * n;

	for (int i = 0; i < d_measureObjs.size(); i++) {
		Measure *measure = d_measureObjs[i];
		measure->setEndIndex(currentIndex);
	}

	value_gateRight = value;
	//find the percentage of the gate in relation with plot width
	double width = (axisScaleDiv(xBottom).upperBound() - value) / (axisScaleDiv(xBottom).upperBound() - axisScaleDiv(xBottom).lowerBound());
	Q_EMIT rightGateChanged(width);
	d_hGatingHandle2->setTimeValue(d_gateBar2->plotCoord().x());

	replot();
}

QWidget * CapturePlot::topArea()
{
	return d_topWidget;
}

QWidget * CapturePlot::topHandlesArea()
{/* handle area for gate cursors */
	return d_topHandlesArea;
}

QWidget * CapturePlot::bottomHandlesArea()
{
	return d_bottomHandlesArea;
}

QWidget * CapturePlot::leftHandlesArea()
{
	return d_leftHandlesArea;
}

QWidget * CapturePlot::rightHandlesArea()
{
	return d_rightHandlesArea;
}

void CapturePlot::setBonusWidthForHistogram(int width)
{
	d_bonusWidth = width;
}

void CapturePlot::setTriggerAEnabled(bool en)
{
	if (d_triggerAEnabled != en) {
		d_triggerAEnabled = en;
		d_levelTriggerABar->setVisible(en);
		d_levelTriggerAHandle->setVisible(en);
	}
}

bool CapturePlot::triggerAEnabled()
{
	return d_triggerAEnabled;
}

void CapturePlot::setTriggerBEnabled(bool en)
{
	if (d_triggerBEnabled != en) {
		d_triggerBEnabled = en;
		d_levelTriggerBBar->setVisible(en);
		d_levelTriggerBHandle->setVisible(en);
	}
}

bool CapturePlot::triggerBEnabled()
{
	return d_triggerBEnabled;
}

void CapturePlot::setVertCursorsEnabled(bool en)
{
	if (d_vertCursorsEnabled != en) {
		d_vertCursorsEnabled = en;
		d_vBar1->setVisible(en);
		d_vBar2->setVisible(en);
		d_hCursorHandle1->setVisible(en);
		d_hCursorHandle2->setVisible(en);
		d_cursorReadouts->setTimeReadoutVisible(en &&
			d_cursorReadoutsVisible);
	}
}

bool CapturePlot::vertCursorsEnabled()
{
	return d_vertCursorsEnabled;
}

void CapturePlot::setHorizCursorsEnabled(bool en)
{
	if (d_horizCursorsEnabled != en) {
		d_horizCursorsEnabled = en;
		d_hBar1->setVisible(en);
		d_hBar2->setVisible(en);
		d_vCursorHandle1->setVisible(en);
		d_vCursorHandle2->setVisible(en);
		d_cursorReadouts->setVoltageReadoutVisible(en &&
			d_cursorReadoutsVisible);
	}
}

bool CapturePlot::horizCursorsEnabled()
{
	return d_horizCursorsEnabled;
}

void CapturePlot::setCursorReadoutsVisible(bool en)
{
	if (d_cursorReadoutsVisible != en) {
		d_cursorReadoutsVisible = en;
		d_cursorReadouts->setVoltageReadoutVisible(en &&
			d_vertCursorsEnabled);
		d_cursorReadouts->setTimeReadoutVisible(en &&
			d_horizCursorsEnabled);
	}
}

void CapturePlot::setSelectedChannel(int id)
{
	if (d_selected_channel != id)  {
		d_selected_channel = id;

		if (id > -1) {
			d_hBar1->setMobileAxis(QwtAxisId(QwtPlot::yLeft, id));
			d_hBar2->setMobileAxis(QwtAxisId(QwtPlot::yLeft, id));
		}
	}
	//
}

int CapturePlot::selectedChannel()
{
	return d_selected_channel;
}

void CapturePlot::setMeasuremensEnabled(bool en)
{
	d_measurementsEnabled = en;
}

bool CapturePlot::measurementsEnabled()
{
	return d_measurementsEnabled;
}

void CapturePlot::onTimeTriggerHandlePosChanged(int pos)
{
	QwtScaleMap xMap = this->canvasMap(QwtAxisId(QwtPlot::xBottom, 0));
	double min = -(xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();
	double max = (xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();

	xMap.setScaleInterval(min, max);
	double time = xMap.invTransform(pos);
	Q_EMIT timeTriggerValueChanged(-time);
}

void CapturePlot::onTimeTriggerHandleGrabbed(bool grabbed) {
	if (grabbed)
		d_timeTriggerBar->setPen(d_timeTriggerActiveLinePen);
	else
		d_timeTriggerBar->setPen(d_timeTriggerInactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::onTriggerAHandleGrabbed(bool grabbed)
{
	if (grabbed)
		d_levelTriggerABar->setPen(d_timeTriggerActiveLinePen);
	else
		d_levelTriggerABar->setPen(d_timeTriggerInactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::onTriggerBHandleGrabbed(bool grabbed)
{
	if (grabbed)
		d_levelTriggerBBar->setPen(d_trigBactiveLinePen);
	else
		d_levelTriggerBBar->setPen(d_trigBinactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::setVertCursorsLocked(bool value)
{
	vertCursorsLocked = value;
}

void CapturePlot::showEvent(QShowEvent *event)
{
	d_vCursorHandle1->triggerMove();
	d_vCursorHandle2->triggerMove();
	d_hCursorHandle1->triggerMove();
	d_hCursorHandle2->triggerMove();
}

void CapturePlot::printWithNoBackground(const QString& toolName, bool editScaleDraw)
{
	QwtPlotMarker detailsMarker;
	detailsMarker.setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	detailsMarker.attach(this);
	double xMarker = axisInterval(QwtPlot::xBottom).maxValue();
	double length = axisInterval(QwtPlot::xBottom).maxValue() - axisInterval(QwtPlot::xBottom).minValue();
	xMarker -= (0.2 * length);
	double yMarker = axisInterval(QwtPlot::yLeft).maxValue();
	yMarker -= (0.1 * yMarker);
	detailsMarker.setValue(xMarker, yMarker);
	QwtText text(d_timeBaseLabel->text() + " " + d_sampleRateLabel->text());
	text.setColor(QColor(0, 0, 0));
	detailsMarker.setLabel(text);
	replot();

	DisplayPlot::printWithNoBackground(toolName, editScaleDraw);
}

int CapturePlot::getAnalogChannels() const
{
	return d_ydata.size() + d_ref_ydata.size();
}

void CapturePlot::setHorizCursorsLocked(bool value)
{
	horizCursorsLocked = value;
}

Measure* CapturePlot::measureOfChannel(int chnIdx) const
{
	Measure *measure = nullptr;

	auto it = std::find_if(d_measureObjs.begin(), d_measureObjs.end(),
		[&](Measure *m) { return m->channel() == chnIdx; });
	if (it != d_measureObjs.end())
		measure = *it;

	return measure;
}

void CapturePlot::bringCurveToFront(unsigned int curveIdx)
{
	for (auto &item : d_offsetHandles) {
		if (item->pen().color() == getLineColor(curveIdx))
			item->raise();
	}

	DisplayPlot::bringCurveToFront(curveIdx);
}

void CapturePlot::enableLabels(bool enabled)
{
	d_labelsEnabled = enabled;
	enableColoredLabels(enabled);
}

void CapturePlot::enableXaxisLabels()
{
	d_bottomHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new XBottomRuller(this)));
}

void CapturePlot::enableAxisLabels(bool enabled)
{
	enableAxis(QwtPlot::xBottom, enabled);
	if (!enabled) {
		int nrAxes = axesCount(QwtPlot::yLeft);
		for (int i = 0; i < nrAxes; ++i) {
			setAxisVisible(QwtAxisId(QwtPlot::yLeft, i),
					enabled);
		}
	}
}

void CapturePlot::setDisplayScale(double value)
{
	DisplayPlot::setDisplayScale(value);
	onVoltageCursor1Moved(d_hBar1->plotCoord().y());
	onVoltageCursor2Moved(d_hBar2->plotCoord().y());
}

void CapturePlot::setTimeTriggerInterval(double min, double max)
{
	d_timeTriggerMinValue = min;
	d_timeTriggerMaxValue = max;
}

bool CapturePlot::labelsEnabled()
{
	return d_labelsEnabled;
}

void CapturePlot::setGraticuleEnabled(bool enabled){
	displayGraticule = enabled;

	if(!displayGraticule){
		for(QwtPlotScaleItem* scale : scaleItems){
			scale->attach(this);
		}
		graticule->enableGraticule(displayGraticule);
	}
	else{
		for(QwtPlotScaleItem* scale : scaleItems){
			scale->detach();
		}
		graticule->enableGraticule(displayGraticule);
	}

	replot();
}

void CapturePlot::setGatingEnabled(bool enabled){
	if(d_gatingEnabled != enabled){
		d_gatingEnabled = enabled;
		d_gateBar1->setVisible(enabled);
		d_gateBar2->setVisible(enabled);
		d_hGatingHandle1->setVisible(enabled);
		d_hGatingHandle2->setVisible(enabled);
		updateHandleAreaPadding(d_labelsEnabled);

		if(enabled){
			leftGate->attach(this);
			rightGate->attach(this);
			d_topHandlesArea->show();
			//update handle
			onGateBar1Moved(leftGateRect.right());
			onGateBar2Moved(rightGateRect.left());
		}
		else{
			leftGate->detach();
			rightGate->detach();
			d_topHandlesArea->hide();
		}
		for (int i = 0; i < d_measureObjs.size(); i++) {
			Measure *measure = d_measureObjs[i];
			measure->setGatingEnabled(enabled);
		}

		d_gateBar1->triggerMove();
		d_gateBar2->triggerMove();

		replot();
	}
}

void CapturePlot::trackModeEnabled(bool enabled)
{
	d_trackMode = !enabled;
	if (d_horizCursorsEnabled) {
		d_hBar1->setVisible(enabled);
		d_hBar2->setVisible(enabled);
		d_vCursorHandle1->setVisible(enabled);
		d_vCursorHandle2->setVisible(enabled);
	}
	if (d_trackMode) {
		onTimeCursor1Moved(d_vBar1->plotCoord().x());
		onTimeCursor2Moved(d_vBar2->plotCoord().x());
		displayIntersection();
	} else {
		onVoltageCursor1Moved(d_hBar1->plotCoord().y());
		onVoltageCursor2Moved(d_hBar2->plotCoord().y());
		markerIntersection1->detach();
		markerIntersection2->detach();
		replot();
	}
}

void CapturePlot::repositionCursors()
{
	onTimeCursor1Moved(d_vBar1->plotCoord().x());
	onTimeCursor2Moved(d_vBar2->plotCoord().x());
	displayIntersection();
}

void CapturePlot::setActiveVertAxis(unsigned int axisIdx, bool selected)
{
	DisplayPlot::setActiveVertAxis(axisIdx, selected);
	updateHandleAreaPadding(d_labelsEnabled);
	if (d_labelsEnabled) {
		enableAxis(QwtPlot::xBottom, true);
	}
}

void CapturePlot::showYAxisWidget(unsigned int axisIdx, bool en)
{
	if (!d_labelsEnabled)
		return;

	setAxisVisible(QwtAxisId(QwtPlot::yLeft, axisIdx),
						en);

	int nrAxes = axesCount(QwtPlot::yLeft);
	bool allAxisDisabled = true;
	for (int i = 0; i < nrAxes; ++i)
		if (isAxisVisible(QwtAxisId(QwtPlot::yLeft, i)))
			allAxisDisabled = false;

	if (allAxisDisabled) {
		setAxisVisible(QwtPlot::xBottom, false);
		updateHandleAreaPadding(false);
	}
	if (en) {
		setAxisVisible(QwtPlot::xBottom, true);
	}
}

void CapturePlot::updateHandleAreaPadding(bool enabled)
{
	double xAxisBonusWidth = 0.0;

	if (axisEnabled(QwtPlot::xBottom)) {
		if (!axisEnabled(QwtPlot::yLeft)) {
			xAxisBonusWidth = 65.0;
		}
	}

	if (enabled) {
		d_bottomHandlesArea->setLeftPadding(50 + axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->width());
		d_topHandlesArea->setLeftPadding(90 + axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->width());
		QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
		const int fmw = QFontMetrics(scaleWidget->font()).width("-XX.XX XX");
		const int fmh = QFontMetrics(scaleWidget->font()).height();
		d_bottomHandlesArea->setRightPadding(50 + fmw/2 + d_bonusWidth);
		d_topHandlesArea->setRightPadding(50 + fmw/2 + d_bonusWidth);
		d_rightHandlesArea->setTopPadding(50 + 6);
		d_rightHandlesArea->setBottomPadding(50 + fmh);
		QMargins margins = d_topWidget->layout()->contentsMargins();
		margins.setLeft(d_leftHandlesArea->minimumWidth()+100);
		d_topWidget->layout()->setContentsMargins(margins);
	} else {
		if(d_topHandlesArea->leftPadding() != 90)
			d_topHandlesArea->setLeftPadding(90);
		if(d_topHandlesArea->rightPadding() != 90)
			d_topHandlesArea->setRightPadding(90);
		if (d_bottomHandlesArea->leftPadding() != 50 + xAxisBonusWidth)
			d_bottomHandlesArea->setLeftPadding(50 + xAxisBonusWidth);
		if (d_bottomHandlesArea->rightPadding() != 50 + d_bonusWidth + xAxisBonusWidth)
			d_bottomHandlesArea->setRightPadding(50 + d_bonusWidth + xAxisBonusWidth);
		if (d_rightHandlesArea->topPadding() != 50)
			d_rightHandlesArea->setTopPadding(50);
		if (d_rightHandlesArea->bottomPadding() != 50)
			d_rightHandlesArea->setBottomPadding(50);

		int topPadding = d_gatingEnabled ? d_topHandlesArea->height() : 0;
		d_leftHandlesArea->setTopPadding(50 + topPadding);
		d_rightHandlesArea->setTopPadding(50 + topPadding);

		QMargins margins = d_topWidget->layout()->contentsMargins();
		margins.setLeft(d_leftHandlesArea->minimumWidth());
		d_topWidget->layout()->setContentsMargins(margins);
	}

	//update handle position to avoid cursors getting out of the plot bounds when changing the padding;
	d_hCursorHandle1->updatePosition();
	d_hCursorHandle2->updatePosition();

	d_vCursorHandle1->updatePosition();
	d_vCursorHandle2->updatePosition();
}

double CapturePlot::getHorizontalCursorIntersection(double time)
{
	int n = Curve(d_selected_channel)->data()->size();

	if (n == 0) {
		return ERROR_VALUE;
	} else {
		double leftTime, rightTime, leftCustom, rightCustom;
		int rightIndex = -1;
		int leftIndex = -1;

		int left = 0;
		int right = n - 1;

		if (Curve(d_selected_channel)->data()->sample(right).x() < time ||
				Curve(d_selected_channel)->data()->sample(left).x() > time) {
			return ERROR_VALUE;
		}

		while (left <= right) {
			int mid = (left + right) / 2;
			double xData = Curve(d_selected_channel)->data()->sample(mid).x();
			if (xData == time) {
				if (mid > 0) {
					leftIndex = mid - 1;
					rightIndex = mid;
				}
				break;
			} else if (xData < time) {
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if ((leftIndex == -1 || rightIndex == -1) && left > 0) {
			leftIndex = left - 1;
			rightIndex = left;
		}

		if (leftIndex == -1 || rightIndex == -1) {
			return ERROR_VALUE;
		}

		leftTime = Curve(d_selected_channel)->data()->sample(leftIndex).x();
		rightTime = Curve(d_selected_channel)->data()->sample(rightIndex).x();

		leftCustom = Curve(d_selected_channel)->data()->sample(leftIndex).y();
		rightCustom = Curve(d_selected_channel)->data()->sample(rightIndex).y();

		double value = (rightCustom - leftCustom) / (rightTime - leftTime) *
				(time - leftTime) + leftCustom;

		return value;
	}
}

void CapturePlot::displayIntersection()
{
	if (!d_trackMode) {
		return;
	}

	double intersectionCursor1, intersectionCursor2;
	bool attachmk1 = true;
	bool attachmk2 = true;


	intersectionCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	intersectionCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());

	if (intersectionCursor1 == -1000000){
		attachmk1 = false;
	}
	if (intersectionCursor2 == -1000000) {
		attachmk2 = false;
	}

	markerIntersection1->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, d_selected_channel));
	markerIntersection2->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, d_selected_channel));

	markerIntersection1->setValue(d_vBar1->plotCoord().x(), intersectionCursor1);
	markerIntersection2->setValue(d_vBar2->plotCoord().x(), intersectionCursor2);

	if (attachmk1) {
		markerIntersection1->attach(this);
	} else {
		markerIntersection1->detach();
	}
	if (attachmk2) {
		markerIntersection2->attach(this);
	} else {
		markerIntersection2->detach();
	}

	replot();
}

void CapturePlot::updateGateMargins(){
	/* update the size of the gates */
	leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	leftGate->setRect(leftGateRect);

	rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	rightGate->setRect(rightGateRect);

	replot();
}

bool CapturePlot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() && event->type() == QEvent::Resize) {
		updateHandleAreaPadding(d_labelsEnabled);

		//force cursor handles to emit position changed
		//when the plot canvas is being resized
		d_hCursorHandle1->triggerMove();
		d_hCursorHandle2->triggerMove();
		d_vCursorHandle1->triggerMove();
		d_vCursorHandle2->triggerMove();

		/* update the size of the gates when the plot canvas is resized */
		updateGateMargins();

		for (int i = 0; i < d_offsetHandles.size(); ++i) {
			d_offsetHandles[i]->triggerMove();
		}

		Q_EMIT canvasSizeChanged();

	}
	return QObject::eventFilter(object, event);
}

QString CapturePlot::getChannelName(int chIdx) const {
	if (chIdx >= d_offsetHandles.size() || chIdx < 0) {
		return "";
	}

	return d_offsetHandles.at(chIdx)->getName();
}

void CapturePlot::setChannelName(const QString &name, int chIdx) {
	if (chIdx >= d_offsetHandles.size() || chIdx < 0) {
		return;
	}

	d_offsetHandles.at(chIdx)->setName(name);
}

void CapturePlot::removeDigitalPlotCurve(QwtPlotCurve *curve)
{
	for (int i = 0; i < d_offsetHandles.size(); ++i) {
		if (curve == getDigitalPlotCurve(i)) {
			removeOffsetWidgets(d_ydata.size() + d_ref_ydata.size() + i);
			removeLeftVertAxis(d_ydata.size() + d_ref_ydata.size() + i);
			break;
		}
	}

	TimeDomainDisplayPlot::removeDigitalPlotCurve(curve);

	replot();
}

void CapturePlot::setOffsetHandleVisible(int chIdx, bool visible)
{
	d_offsetHandles.at(chIdx)->setVisible(visible);
}

void CapturePlot::addToGroup(int currentGroup, int toAdd)
{
	const bool selected = d_offsetHandles.at(currentGroup)->isSelected();
	d_offsetHandles.at(currentGroup)->selected(true);
	d_offsetHandles.at(currentGroup)->setSelected(true);

	beginGroupSelection();

	d_offsetHandles.at(toAdd)->selected(true);
	endGroupSelection(true);

	d_offsetHandles.at(currentGroup)->setSelected(selected);
	d_offsetHandles.at(currentGroup)->selected(selected);
}

void CapturePlot::onDigitalChannelAdded(int chnIdx)
{
	setLeftVertAxesCount(d_ydata.size() + d_ref_ydata.size() + chnIdx + 1);
	setAxisScale( QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx), -5, 5);
	replot();

	QColor chnColor;
	const int h = (55 * chnIdx) % 360;
	const int s = 180;
	const int v = 170;
	chnColor.setHsl(h, s, v);

	QwtPlotCurve *curve = getDigitalPlotCurve(chnIdx);
	GenericLogicPlotCurve *logicCurve = dynamic_cast<GenericLogicPlotCurve *>(curve);

	curve->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx));

	/* Channel offset widget */
	HorizBar *chOffsetBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(chOffsetBar);
	chOffsetBar->setCanLeavePlot(true);
	chOffsetBar->setVisible(false);
	chOffsetBar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx));
	d_offsetBars.push_back(chOffsetBar);

	RoundedHandleV *chOffsetHdl = new RoundedHandleV(
				QPixmap(":/icons/handle_right_arrow.svg"),
				QPixmap(":/icons/handle_up_arrow.svg"),
				QPixmap(":/icons/handle_down_arrow.svg"),
				d_leftHandlesArea, true, logicCurve->getName(), true); // TODO: add name
	chOffsetHdl->setRoundRectColor(chnColor);
	chOffsetHdl->setPen(QPen(chnColor, 2, Qt::SolidLine));
	chOffsetHdl->setVisible(true);
	d_offsetHandles.push_back(chOffsetHdl);

	connect(logicCurve, &GenericLogicPlotCurve::nameChanged,
		[=](const QString &name) {
		chOffsetHdl->setName(name);
	});

	connect(logicCurve, &GenericLogicPlotCurve::pixelOffsetChanged, [=](double offset) {
		chOffsetBar->setPosition(offset);
	});

	connect(chOffsetHdl, &RoundedHandleV::selected, [=](bool selected){
		Q_EMIT channelSelected(d_offsetHandles.indexOf(chOffsetHdl), selected);

		if (!selected && !d_startedGrouping) {
			return ;
		}

		if (d_startedGrouping) {
			chOffsetHdl->setSelected(selected);
			if (selected) {
				d_groupHandles.back().push_back(chOffsetHdl);
			} else {
				d_groupHandles.back().removeOne(chOffsetHdl);
			}
			return;
		}

		for (auto &hdl : d_offsetHandles) {
			if (hdl == chOffsetHdl) {
				continue;
			}

			hdl->setSelected(false);
		}
	});

	/* When bar position changes due to plot resizes update the handle */
	connect(chOffsetBar, &HorizBar::pixelPositionChanged,
		[=](int pos) {
			chOffsetHdl->setPositionSilenty(pos);
		});

	connect(chOffsetHdl, &RoundedHandleV::positionChanged,
		[=](int pos) {
			int chn_id = d_offsetHandles.indexOf(chOffsetHdl);
			if (chn_id < 0)
				return;

//			qDebug() << pos;

			QwtScaleMap yMap = this->canvasMap(QwtAxisId(QwtPlot::yLeft, chn_id));

			auto y = axisInterval(QwtAxisId(QwtPlot::yLeft, chn_id));

//			double min = -(yAxisNumDiv() / 2.0) * VertUnitsPerDiv(0);
//			double max = (yAxisNumDiv() / 2.0) * VertUnitsPerDiv(0);

			double min = y.minValue();
			double max = y.maxValue();

//			qDebug() << min << " " << max;

			yMap.setScaleInterval(min, max);
			double offset = yMap.invTransform(pos);

			QwtPlotCurve *curve = getDigitalPlotCurve(chnIdx);
			GenericLogicPlotCurve *logicCurve = dynamic_cast<GenericLogicPlotCurve *>(curve);

			if (logicCurve) {
//				double plotOffset = VertOffset(0);
				double pixelOffset = offset /*- logicCurve->getTraceHeight() / 2.0*/;
				logicCurve->setPixelOffset(pixelOffset /*+ plotOffset*/);
				QSignalBlocker blocker(chOffsetBar);
				chOffsetBar->setPosition(pixelOffset /*+ plotOffset*/);
			}


			replot();

//			Q_EMIT channelOffsetChanged(-offset);
		});

		chOffsetHdl->setPosition(d_currentHandleInitPx);
		d_currentHandleInitPx += 20;
}

void CapturePlot::setChannelSelectable(int chnIdx, bool selectable)
{
	d_offsetHandles.at(chnIdx)->setSelectable(selectable);
}

void CapturePlot::beginGroupSelection()
{
	if (d_startedGrouping) {
		qDebug() << "\"beginGroupSelection\" already called. Consider terminating current group"
			    "creation using \"endGroupSelection\"!";
	}

	d_startedGrouping = true;

	d_groupHandles.push_back(QList<RoundedHandleV *>());
	for (auto &hdl : d_offsetHandles) {
		if (hdl->isSelected()) {
			d_groupHandles.back().push_back(hdl);
		}
	}
}

bool CapturePlot::endGroupSelection(bool moveAnnotationCurvesLast)
{
	if (!d_startedGrouping) {
		qDebug() << "\"endGroupSelection\" call not paired with \"beginGroupSelection\"!";
		return false;
	}

	d_startedGrouping = false;

	if (d_groupHandles.back().size() < 2) {
		d_groupHandles.pop_back();
		return false;
	}

	for (auto & grp : d_groupHandles) {
		for (RoundedHandleV *hdl : grp) {
			disconnect(hdl, &RoundedHandleV::positionChanged,
				this, &CapturePlot::handleInGroupChangedPosition);
		}
	}

	// merge new group if selected channels already have a group
	QList<RoundedHandleV *> group = d_groupHandles.takeLast();
	QList<RoundedHandleV *> updatedGroup;
	for (RoundedHandleV *hdl : group) {
		auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
					     [&hdl](const QList<RoundedHandleV*> &group){
			return group.contains(hdl);
		});

		if (hdlGroup == d_groupHandles.end()) {
			if (!updatedGroup.contains(hdl)) {
				updatedGroup.push_back(hdl);
			}
			continue;
		}

		for (const auto &grpHdl : *hdlGroup) {
			if (!updatedGroup.contains(grpHdl)) {
				updatedGroup.push_back(grpHdl);
			}
		}

		d_groupHandles.removeOne(*hdlGroup);
	}
	group = updatedGroup;
	//
	if (moveAnnotationCurvesLast) {
		std::sort(group.begin(), group.end(), [=](RoundedHandleV *a, RoundedHandleV *b){
			return d_offsetHandles.indexOf(a) < d_offsetHandles.indexOf(b);
		});
	}

	d_groupHandles.push_back(group);

	auto getTraceHeightInPixelsForHandle = [=](RoundedHandleV *handle, double &bonusHeight) {
		const int chIdx = d_offsetHandles.indexOf(handle);
		QwtPlotCurve *curve = getDigitalPlotCurve(chIdx);
		GenericLogicPlotCurve *logicCurve = dynamic_cast<GenericLogicPlotCurve *>(curve);
		AnnotationCurve *annCurve = dynamic_cast<AnnotationCurve*>(logicCurve);
		if (annCurve) {
			bonusHeight = annCurve->getTraceHeight() * annCurve->getVisibleRows();
			return static_cast<double>(handle->size().height());
		}
		bonusHeight = 0.0;
		return logicCurve->getTraceHeight();
	};

	const bool newGroup = (d_groupHandles.size() != d_groupMarkers.size());


	if (newGroup) {
		// Move channels on top side of the plot
		int currentPos = 5;
		double bonusHeight = 0.0;
		currentPos += getTraceHeightInPixelsForHandle(group.first(), bonusHeight);
		group.first()->setPosition(currentPos);
		for (int i = 1; i < group.size(); ++i) {
			currentPos += bonusHeight;
			currentPos += getTraceHeightInPixelsForHandle(group[i], bonusHeight);
			group[i]->setPosition(currentPos);
			currentPos += 5;

		}
	}

	for (auto & grp : d_groupHandles) {
		for (RoundedHandleV *hdl : grp) {
			connect(hdl, &RoundedHandleV::positionChanged,
				this, &CapturePlot::handleInGroupChangedPosition);
			hdl->setSelected(false);
		}
	}

	group.first()->setSelected(true);
	group.first()->selected(true);

	for (QwtPlotZoneItem *groupMarker : d_groupMarkers) {
		groupMarker->detach();
		delete groupMarker;
	}

	d_groupMarkers.clear();

	for (const auto &group : d_groupHandles) {
		// Add group marker
		QwtScaleMap yMap = this->canvasMap(QwtAxisId(QwtPlot::yLeft, 0));
		const QwtInterval y = axisInterval(QwtAxisId(QwtPlot::yLeft, 0));
		const double min = y.minValue();
		const double max = y.maxValue();
		yMap.setScaleInterval(min, max);
		double bonusHeight = 0.0;
		double y1 = yMap.invTransform(group.front()->position() -
						    getTraceHeightInPixelsForHandle(group.front(), bonusHeight) - 5);
		getTraceHeightInPixelsForHandle(group.back(), bonusHeight);
		double y2 = yMap.invTransform(group.back()->position() + 5 + bonusHeight);

		QwtPlotZoneItem *groupMarker = new QwtPlotZoneItem();
		d_groupMarkers.push_back(groupMarker);
		groupMarker->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, 0));
		groupMarker->setPen(QColor(74, 100, 255, 30), 2.0);
		groupMarker->setBrush(QBrush(QColor(74, 100, 255, 10)));
		groupMarker->setInterval(y2, y1);
		groupMarker->setOrientation(Qt::Horizontal);
		groupMarker->attach(this);
	}

	if (!newGroup) {
		for (const auto &group : d_groupHandles) {
			group.first()->triggerMove();
		}
	}

	return true;
}

QVector<int> CapturePlot::getGroupOfChannel(int chnIdx)
{
	QVector<int> groupIdxList;

	if (chnIdx < 0 || chnIdx >= d_offsetHandles.size()) {
		return groupIdxList; // empty
	}

	auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
				     [=](const QList<RoundedHandleV*> &group){
		return group.contains(d_offsetHandles[chnIdx]);
	});

	// if no group return
	if (hdlGroup == d_groupHandles.end()) {
		return groupIdxList;
	}

	for (const auto &hdl : *hdlGroup) {
		groupIdxList.push_back(d_offsetHandles.indexOf(hdl));
	}

	return groupIdxList;

}

QVector<QVector<int> > CapturePlot::getAllGroups()
{
	QVector<QVector<int>> allGroups;
	for (int i = 0; i < d_groupHandles.size(); ++i) {
		QVector<int> group;
		for (int j = 0; j < d_groupHandles[i].size(); ++j) {
			int ch = d_offsetHandles.indexOf(d_groupHandles[i][j]);
			if (ch != -1) {
				group.append(ch);
			}
		}
		if (!group.empty()) {
			allGroups.append(group);
		}
	}

	return allGroups;
}

void CapturePlot::removeFromGroup(int chnIdx, int removedChnIdx, bool &didGroupVanish)
{
	auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
				     [=](const QList<RoundedHandleV*> &group){
		return group.contains(d_offsetHandles[chnIdx]);
	});

	if (hdlGroup == d_groupHandles.end()) {
		return;
	}

	const int positionOfFirstHandleInGroup = hdlGroup->at(0)->position();
	const int positionOfLastHandleInGroup = hdlGroup->back()->position();

	disconnect(hdlGroup->at(removedChnIdx), &RoundedHandleV::positionChanged,
		   this, &CapturePlot::handleInGroupChangedPosition);

	auto removedObj = hdlGroup->takeAt(removedChnIdx);

	if (hdlGroup->size() < 2) {
		didGroupVanish = true;
		disconnect(hdlGroup->first(), &RoundedHandleV::positionChanged,
			   this, &CapturePlot::handleInGroupChangedPosition);
		const int indexOfCurrentGroup = d_groupHandles.indexOf(*hdlGroup);
		d_groupHandles.removeOne(*hdlGroup);
		auto marker = d_groupMarkers.takeAt(indexOfCurrentGroup);
		marker->detach();
		delete marker;

		replot();

		return;
	}

	hdlGroup->at(0)->setPosition(positionOfFirstHandleInGroup);
	hdlGroup->at(0)->triggerMove();

	removedObj->setPosition(positionOfLastHandleInGroup);
	removedObj->triggerMove();
}

void CapturePlot::positionInGroupChanged(int chnIdx, int from, int to)
{
	auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
				     [=](const QList<RoundedHandleV*> &group){
		return group.contains(d_offsetHandles[chnIdx]);
	});

	if (hdlGroup == d_groupHandles.end()) {
		return;
	}

	const int positionOfFirstHandleInGroup = hdlGroup->at(0)->position();

	auto item = hdlGroup->takeAt(from);
	hdlGroup->insert(to, item);

	hdlGroup->at(0)->setPosition(positionOfFirstHandleInGroup);
	hdlGroup->at(0)->triggerMove();
}

void CapturePlot::setGroups(const QVector<QVector<int> > &groups)
{
	auto selectedHandleIt = std::find_if(d_offsetHandles.begin(), d_offsetHandles.end(),
					  [](RoundedHandleV *handle){
		return handle->isSelected();
	});

	if (selectedHandleIt != d_offsetHandles.end()) {
		(*selectedHandleIt)->setSelected(false);
		(*selectedHandleIt)->selected(false);
	}

	for (const auto &grp : groups) {
		if (grp.size() < 2) { continue; }
		beginGroupSelection();
		for (const auto &hdl : grp) {
			d_groupHandles.back().push_back(d_offsetHandles.at(hdl));
		}
		endGroupSelection();

		d_groupHandles.back().front()->setSelected(false);
		d_groupHandles.back().front()->selected(false);
		d_groupHandles.back().front()->setPosition(d_groupHandles.back().front()->position());
	}

	if (selectedHandleIt != d_offsetHandles.end()) {
		(*selectedHandleIt)->setSelected(true);
		(*selectedHandleIt)->selected(true);
	}

	replot();
}

void CapturePlot::handleInGroupChangedPosition(int position)
{
	RoundedHandleV *hdl = dynamic_cast<RoundedHandleV *>(QObject::sender());

	// is the sender a RoundedHandleV?
	if (!hdl) {
		qDebug() << "Invalid sender!";
	}

	// find the group of this handle
	auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
				     [&hdl](const QList<RoundedHandleV*> &group){
		return group.contains(hdl);
	});

	// if no group return
	if (hdlGroup == d_groupHandles.end()) {
		return;
	}

	// index of handle in the group
	const int index = hdlGroup->indexOf(hdl);
	const int groupIndex = d_groupHandles.indexOf(*hdlGroup);

	auto getTraceHeightInPixelsForHandle = [=](RoundedHandleV *handle, double &bonusHeight) {
		const int chIdx = d_offsetHandles.indexOf(handle);
		QwtPlotCurve *curve = getDigitalPlotCurve(chIdx);
		GenericLogicPlotCurve *logicCurve = dynamic_cast<GenericLogicPlotCurve *>(curve);
		AnnotationCurve *annCurve = dynamic_cast<AnnotationCurve*>(logicCurve);
		if (annCurve) {
			bonusHeight = annCurve->getTraceHeight() * annCurve->getVisibleRows();
			return static_cast<double>(handle->size().height());
		}
		bonusHeight = 0.0;
		return logicCurve->getTraceHeight();
	};

	// update position of handles above the moved one
	double bonusHeight = 0.0;
	int currentPos = position - 5;
	currentPos -= getTraceHeightInPixelsForHandle(hdl, bonusHeight);
	for (int i = index - 1; i >= 0; --i) {
		if (!hdlGroup->at(i)->isVisible()) {
			continue;
		}
		disconnect(hdlGroup->at(i), &RoundedHandleV::positionChanged,
			   this, &CapturePlot::handleInGroupChangedPosition);
		hdlGroup->at(i)->setPosition(currentPos);
		int temp = getTraceHeightInPixelsForHandle(hdlGroup->at(i), bonusHeight);
		if (bonusHeight != 0.0) {
			hdlGroup->at(i)->setPosition(currentPos - bonusHeight);
			currentPos -= bonusHeight;
			bonusHeight = 0.0;
		}
		currentPos -= temp;
		currentPos -= 5;
		connect(hdlGroup->at(i), &RoundedHandleV::positionChanged,
			   this, &CapturePlot::handleInGroupChangedPosition);
	}

	// update position of handles below the moved one
	currentPos = position + 5;
	getTraceHeightInPixelsForHandle(hdl, bonusHeight);
	for (int i = index + 1; i < hdlGroup->size(); ++i) {
		if (!hdlGroup->at(i)->isVisible()) {
			continue;
		}
		if (bonusHeight != 0.0) {
			currentPos += bonusHeight;
			bonusHeight = 0.0;
		}
		currentPos += getTraceHeightInPixelsForHandle(hdlGroup->at(i), bonusHeight);
		disconnect(hdlGroup->at(i), &RoundedHandleV::positionChanged,
			   this, &CapturePlot::handleInGroupChangedPosition);
		hdlGroup->at(i)->setPosition(currentPos);
		connect(hdlGroup->at(i), &RoundedHandleV::positionChanged,
			   this, &CapturePlot::handleInGroupChangedPosition);
		currentPos += 5;
	}

	// update plot marker
	QwtScaleMap yMap = this->canvasMap(QwtAxisId(QwtPlot::yLeft, 0));
	const QwtInterval y = axisInterval(QwtAxisId(QwtPlot::yLeft, 0));
	const double min = y.minValue();
	const double max = y.maxValue();
	yMap.setScaleInterval(min, max);

	RoundedHandleV *bottomHandle = nullptr, *topHandle = nullptr;
	for (int i = 0; i < hdlGroup->size(); ++i) {
		if (hdlGroup->at(i)->isVisible()) {
			topHandle = hdlGroup->at(i);
			break;
		}
	}
	for (int i = hdlGroup->size() - 1; i >= 0; --i) {
		if (hdlGroup->at(i)->isVisible()) {
			bottomHandle = hdlGroup->at(i);
			break;
		}
	}

	if (bottomHandle && topHandle) {
		double y1 = yMap.invTransform(topHandle->position() -
						    getTraceHeightInPixelsForHandle(topHandle, bonusHeight) - 5);
		getTraceHeightInPixelsForHandle(bottomHandle, bonusHeight);
		double y2 = yMap.invTransform(bottomHandle->position() + 5 + bonusHeight);

		d_groupMarkers.at(groupIndex)->setInterval(y2, y1);
	}

	replot();
}

void adiscope::CapturePlot::pushBackNewOffsetWidgets(RoundedHandleV *chOffsetHdl, HorizBar *chOffsetBar)
{
	int indexOfNewChannel = d_ydata.size() - 1;
	d_offsetBars.insert(indexOfNewChannel, chOffsetBar);
	d_offsetHandles.insert(indexOfNewChannel, chOffsetHdl);

	for (int i = 0; i < d_offsetBars.size(); ++i) {
		d_offsetBars[i]->setMobileAxis(QwtAxisId(QwtPlot::yLeft, i));
	}

	for (int i = 0; i < d_logic_curves.size(); ++i) {
		d_logic_curves[i]->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + i));
	}
}

void CapturePlot::onChannelAdded(int chnIdx)
{
	setLeftVertAxesCount(d_offsetHandles.size() + 1);
	QColor chnColor = getLineColor(chnIdx);

	/* Channel offset widget */
	HorizBar *chOffsetBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(chOffsetBar);
	chOffsetBar->setCanLeavePlot(true);
	chOffsetBar->setVisible(false);
	chOffsetBar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, chnIdx));

	RoundedHandleV *chOffsetHdl = new RoundedHandleV(
				QPixmap(":/icons/handle_right_arrow.svg"),
				QPixmap(":/icons/handle_up_arrow.svg"),
				QPixmap(":/icons/handle_down_arrow.svg"),
				d_leftHandlesArea, true);
	chOffsetHdl->setRoundRectColor(chnColor);
	chOffsetHdl->setPen(QPen(chnColor, 2, Qt::SolidLine));
	chOffsetHdl->setVisible(true);
	pushBackNewOffsetWidgets(chOffsetHdl, chOffsetBar);

	connect(chOffsetHdl, &RoundedHandleV::positionChanged,
		[=](int pos) {
			int chn_id = d_offsetHandles.indexOf(chOffsetHdl);
			if (chn_id < 0)
				return;

			QwtScaleMap yMap = this->canvasMap(QwtAxisId(QwtPlot::yLeft, chn_id));
			double min = -(yAxisNumDiv() / 2.0) * VertUnitsPerDiv(chn_id);
			double max = (yAxisNumDiv() / 2.0) * VertUnitsPerDiv(chn_id);

			yMap.setScaleInterval(min, max);
			double offset = yMap.invTransform(pos);
			this->setVertOffset(-offset, chn_id);
			this->replot();

			Q_EMIT channelOffsetChanged(chn_id, -offset);
		});
	/* When bar position changes due to plot resizes update the handle */
	connect(chOffsetBar, &HorizBar::pixelPositionChanged,
		[=](int pos) {
			chOffsetHdl->setPositionSilenty(pos);
		});

	connect(chOffsetHdl, &RoundedHandleV::mouseReleased,
		[=](){
			int chn_id = d_offsetHandles.indexOf(chOffsetHdl);
			int offset = this->VertOffset(chn_id);
			if (offset > d_maxOffsetValue){
				offset = d_maxOffsetValue;
				this->setVertOffset(offset, chn_id);
				this->replot();

				Q_EMIT channelOffsetChanged(chn_id, offset);
			}
			if (offset < d_minOffsetValue){
				offset = d_minOffsetValue;
				this->setVertOffset(offset, chn_id);
				this->replot();

				Q_EMIT channelOffsetChanged(chn_id, offset);
			}
	});

	connect(chOffsetHdl, &RoundedHandleV::reset, [=](){
		int chn_id = d_offsetHandles.indexOf(chOffsetHdl);
		this->setVertOffset(0, chn_id);
		this->replot();

		Q_EMIT channelOffsetChanged(chn_id, 0);
	});

	/* Add Measure ojbect that handles all channel measurements */

	Measure *measure = nullptr;

	if (isReferenceWaveform(Curve(chnIdx))) {
		int idx = chnIdx - d_ydata.size();
		measure = new Measure(chnIdx, d_ref_ydata[idx],
			Curve(chnIdx)->data()->size(), nullptr);
	} else {
		int count = countReferenceWaveform(chnIdx);
		measure = new Measure(chnIdx, d_ydata[chnIdx - count],
			Curve(chnIdx)->data()->size(), m_conversion_function);
	}

	measure->setAdcBitCount(12);
	d_measureObjs.push_back(measure);
}

void CapturePlot::computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate)
{
	if (chnIdx >= d_measureObjs.size()) {
		return;
	}

	Measure *measure = d_measureObjs[chnIdx];
	measure->setSampleRate(sampleRate);
	measure->measure();

	Q_EMIT measurementsAvailable();
}

void CapturePlot::setConversionFunction(const std::function<double(unsigned int, double, bool)> &fp)
{
	m_conversion_function = fp;
	for (int i = 0; i < d_measureObjs.size(); i++) {
		Measure *measure = d_measureObjs[i];
		measure->setConversionFunction(fp);
	}
}

void CapturePlot::cleanUpJustBeforeChannelRemoval(int chnIdx)
{
	Measure *measure = measureOfChannel(chnIdx);
	if (measure) {
		int pos = d_measureObjs.indexOf(measure);
		for (int i = pos + 1; i < d_measureObjs.size(); i++) {
			d_measureObjs[i]->setChannel(
				d_measureObjs[i]->channel() - 1);
		}
		d_measureObjs.removeOne(measure);
		delete measure;
	}
}

void CapturePlot::setOffsetWidgetVisible(int chnIdx, bool visible)
{
	if (chnIdx < 0 || chnIdx >= d_offsetHandles.size())
		return;

	d_offsetHandles[chnIdx]->setVisible(visible);

	// find the group of this handle
	auto hdlGroup = std::find_if(d_groupHandles.begin(), d_groupHandles.end(),
				     [=](const QList<RoundedHandleV*> &group){
		return group.contains(d_offsetHandles[chnIdx]);
	});

	// if no group return
	if (hdlGroup == d_groupHandles.end()) {
		qDebug() << "This handle is not in a group!";
		return;
	}

	int count = 0;
	for (const auto &handle : *hdlGroup) {
		if (handle->isVisible()) {
			count++;
		}
	}

	const bool detach = (count < 2);
	const int groupIdx = d_groupHandles.indexOf(*hdlGroup);
	if (detach) {
		d_groupMarkers[groupIdx]->detach();
	} else {
		d_groupMarkers[groupIdx]->attach(this);
	}
}

void CapturePlot::removeOffsetWidgets(int chnIdx)
{
	if (chnIdx < 0 || chnIdx >= d_offsetHandles.size())
		return;

	HorizBar *bar = d_offsetBars.takeAt(chnIdx);
	bar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, 0));
	d_symbolCtrl->detachSymbol(bar);
	delete bar;
	delete(d_offsetHandles.takeAt(chnIdx));
}

void CapturePlot::measure()
{
	for (int i = 0; i < d_measureObjs.size(); i++) {
		Measure *measure = d_measureObjs[i];
		if (measure->activeMeasurementsCount() > 0) {
			measure->setSampleRate(this->sampleRate());
			measure->measure();
		}
	}
}

int CapturePlot::activeMeasurementsCount(int chnIdx)
{
	int count = -1;
	Measure *measure = measureOfChannel(chnIdx);

	if (measure)
		count = measure->activeMeasurementsCount();

	return count;
}

void CapturePlot::onNewDataReceived()
{
	int ref_idx = 0;
	for (int i = 0; i < d_measureObjs.size(); i++) {
		Measure *measure = d_measureObjs[i];
		int chn = measure->channel();
		if (isReferenceWaveform(Curve(chn))) {
			measure->setDataSource(d_ref_ydata[ref_idx],
					       Curve(chn)->data()->size());
			ref_idx++;
		} else {
			int count = countReferenceWaveform(chn);
			measure->setDataSource(d_ydata[chn - count],
					Curve(chn)->data()->size());
		}

		if (isMathWaveform(Curve(chn))) {
			measure->setAdcBitCount(0);
		}

		measure->setSampleRate(this->sampleRate());
		measure->measure();
	}

	Q_EMIT measurementsAvailable();
}

QList<std::shared_ptr<MeasurementData>> CapturePlot::measurements(int chnIdx)
{
	Measure *measure = measureOfChannel(chnIdx);

	if (measure)
		return measure->measurments();
	else
		return QList<std::shared_ptr<MeasurementData>>();
}

std::shared_ptr<MeasurementData> CapturePlot::measurement(int id, int chnIdx)
{
	Measure *measure = measureOfChannel(chnIdx);
	if (measure)
		return measure->measurement(id);
	else
		return std::shared_ptr<MeasurementData>();
}

OscPlotZoomer *CapturePlot::getZoomer()
{
	if (d_zoomer.isEmpty())
		return nullptr;
	return static_cast<OscPlotZoomer* >(d_zoomer[0]);
}

void CapturePlot::setOffsetInterval(double minValue, double maxValue)
{
	d_minOffsetValue = minValue;
	d_maxOffsetValue = maxValue;
}

double CapturePlot::getMaxOffsetValue()
{
	return d_maxOffsetValue;
}

double CapturePlot::getMinOffsetValue()
{
	return d_minOffsetValue;
}

void CapturePlot::setPeriodDetectLevel(int chnIdx, double lvl)
{
	Measure *measure = measureOfChannel(chnIdx);
	if (measure)
		measure->setCrossLevel(lvl);
}

void CapturePlot::setPeriodDetectHyst(int chnIdx, double hyst)
{
	Measure *measure = measureOfChannel(chnIdx);
	if (measure)
		measure->setHysteresisSpan(hyst);
}

struct cursorReadoutsText CapturePlot::allCursorReadouts() const
{
	return d_cursorReadoutsText;
}

void CapturePlot::setTimeBaseLabelValue(double value)
{
	QString text = d_cursorTimeFormatter.format(value, "", 3);
	if (d_timeBaseLabel->text().contains(tr("Zoom: "))) {
		d_timeBaseLabel->setText(tr("Zoom: ") + text + tr("/div"));
	} else {
		d_timeBaseLabel->setText(text + tr("/div"));
	}
}

void CapturePlot::setTimeBaseZoomed(bool zoomed)
{
	if (zoomed) {
		if (!d_timeBaseLabel->text().contains(tr("Zoom: ")))
			d_timeBaseLabel->setText(tr("Zoom: ") + d_timeBaseLabel->text());
	} else {
		QString text = d_timeBaseLabel->text();
		if (text.contains(tr("Zoom: "))) {
			text = text.remove(tr("Zoom: "));
			d_timeBaseLabel->setText(text);
		}
	}
}

void CapturePlot::setBufferSizeLabelValue(int numSamples)
{
	d_bufferSizeLabelVal = numSamples;
	updateBufferSizeSampleRateLabel(numSamples, d_sampleRateLabelVal);
}

void CapturePlot::setSampleRatelabelValue(double sampleRate)
{
	d_sampleRateLabelVal = sampleRate;
	updateBufferSizeSampleRateLabel(d_bufferSizeLabelVal, sampleRate);
}

void CapturePlot::setTriggerState(int triggerState)
{
	d_triggerStateLabel->hide();

	switch (triggerState) {
	case Waiting:
		d_triggerStateLabel->setText(tr("Waiting"));
		break;
	case Triggered:
		d_triggerStateLabel->setText(tr("Triggered"));
		break;
	case Stop:
		d_triggerStateLabel->setText(tr("Stop"));
		break;
	case Auto:
		d_triggerStateLabel->setText(tr("Auto"));
		break;
	default:
		break;
	};
	d_triggerStateLabel->show();
}

void CapturePlot::setMaxBufferSizeErrorLabel(bool reached, const QString &customWarning)
{
	QString errorMessage = "Maximum buffer size reached";
	if (customWarning.length()) {
		errorMessage = customWarning;
	}
	d_maxBufferError->setText(reached ? errorMessage : "");
}

void CapturePlot::setCursorReadoutsTransparency(int value)
{
	d_cursorReadouts->setTransparency(value);
}

void CapturePlot::moveCursorReadouts(CustomPlotPositionButton::ReadoutsPosition position)
{
	d_cursorReadouts->moveToPosition(position);
}

void CapturePlot::updateBufferSizeSampleRateLabel(int nsamples, double sr)
{
	QString txtSampleRate = d_cursorMetricFormatter.format(sr, "sps", 0);
	d_cursorMetricFormatter.setTrimZeroes(true);
	QString txtSamples = d_cursorMetricFormatter.format(nsamples, "", 3);
	d_cursorMetricFormatter.setTrimZeroes(false);
	QString text = QString("%1 Samples at ").arg(txtSamples) + txtSampleRate;
	d_sampleRateLabel->setText(text);
}

void CapturePlot::removeLeftVertAxis(unsigned int axis)
{
	const unsigned int numAxis = vertAxes.size();

	if (axis >= numAxis)
		return;

	// Update the mobile axis ID of all symbols
	for (int i = axis; i < numAxis - 1; i++) {
		QwtAxisId axisId = d_offsetBars.at(i)->mobileAxis();
		--axisId.id;
		d_offsetBars.at(i)->setMobileAxis(axisId);
	}

	DisplayPlot::removeLeftVertAxis(axis);
}


