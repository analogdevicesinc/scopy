/*
 * Copyright 2016 Analog Devices, Inc.
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

#include "oscilloscope_plot.hpp"
#include "symbol_controller.h"
#include "handles_area.hpp"
#include "plot_line_handle.h"

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
	setSampleRate(100E6, 1, "");

	QFile file(":/stylesheets/stylesheets/plot.qss");
	file.open(QFile::ReadOnly);
	QString stylesheet = QString::fromLatin1(file.readAll());
	setStyleSheet(stylesheet);

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
	d_measurementEnabled(false)
{
	/* Initial colors scheme */
	d_trigAactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	d_trigAinactiveLinePen = QPen(QColor(175, 175, 175), 2, Qt::DashLine);
	d_trigBactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	d_trigBinactiveLinePen = QPen(QColor(175, 175, 175), 2, Qt::DashLine);
	/* End of: Initial colors scheme */

	setSymmetricDataEnabled(true);

	d_symbolCtrl = new SymbolController(this);

	setHorizUnitsPerDiv(1E-6);
	zoomBaseUpdate();

	/* Adjacent areas (top/bottom/left/right) */
	d_topWidget = new QWidget(this);
	d_bottomHandlesArea = new HorizHandlesArea(this->canvas());
	d_leftHandlesArea = new VertHandlesArea(this->canvas());
	d_rightHandlesArea = new VertHandlesArea(this->canvas());

	d_topWidget->setStyleSheet("QWidget {background-color: transparent}");
	d_topWidget->setMinimumHeight(50);
	d_bottomHandlesArea->setMinimumHeight(50);
	d_leftHandlesArea->setMinimumWidth(50);
	d_rightHandlesArea->setMinimumWidth(50);
	d_bottomHandlesArea->setLargestChildWidth(60);
	d_rightHandlesArea->setLargestChildHeight(60);

	/* Time trigger widget */
	d_timeTriggerBar = new VertBar(this);
	d_symbolCtrl->attachSymbol(d_timeTriggerBar);
	d_timeTriggerBar->setPen(QPen(QColor(74, 100, 255), 2, Qt::SolidLine));
	d_timeTriggerBar->setCanLeavePlot(true);

	d_timeTriggerHandle = new FreePlotLineHandleH(
					QPixmap(":/icons/time_trigger_handle.svg"),
					QPixmap(":/icons/time_trigger_left.svg"),
					QPixmap(":/icons/time_trigger_right.svg"),
					d_bottomHandlesArea);
	d_timeTriggerHandle->setPen(d_timeTriggerBar->pen());

	connect(d_timeTriggerHandle, SIGNAL(positionChanged(int)),
		SLOT(onTimeTriggerHandlePosChanged(int)));

	/* When bar position changes due to plot resizes update the handle */
	connect(d_timeTriggerBar, &VertBar::pixelPositionChanged,
		[=](int pos) {
			d_timeTriggerHandle->setPositionSilenty(pos);
		});

	/* Level triggers widgets */
	// Trigger A
	d_levelTriggerABar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(d_levelTriggerABar);
	d_levelTriggerABar->setPen(d_trigAinactiveLinePen);
	d_levelTriggerABar->setCanLeavePlot(true);

	d_levelTriggerAHandle = new FreePlotLineHandleV(
					QPixmap(":/icons/level_trigger_handle.svg"),
					QPixmap(":/icons/level_trigger_up.svg"),
					QPixmap(":/icons/level_trigger_down.svg"),
					d_rightHandlesArea);
	d_levelTriggerAHandle->setPen(d_trigAactiveLinePen);

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

	d_vBar1 = new VertBar(this, true);
	d_vBar2 = new VertBar(this, true);
	d_hBar1 = new HorizBar(this, true);
	d_hBar2 = new HorizBar(this, true);

	d_symbolCtrl->attachSymbol(d_vBar1);
	d_symbolCtrl->attachSymbol(d_vBar2);
	d_symbolCtrl->attachSymbol(d_hBar1);
	d_symbolCtrl->attachSymbol(d_hBar2);

	QPen cursorsLinePen = QPen(QColor(155, 155, 155), 1, Qt::DashLine);
	d_hBar1->setPen(cursorsLinePen);
	d_hBar2->setPen(cursorsLinePen);
	d_vBar1->setPen(cursorsLinePen);
	d_vBar2->setPen(cursorsLinePen);

	d_vCursorHandle1->setPen(cursorsLinePen);
	d_vCursorHandle2->setPen(cursorsLinePen);
	d_hCursorHandle1->setPen(cursorsLinePen);
	d_hCursorHandle2->setPen(cursorsLinePen);

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
	d_hBar1->setPosition(0 + voltsPerDiv);
	d_hBar2->setPosition(0 - voltsPerDiv);
	d_vBar1->setPosition(0 + secPerDiv);
	d_vBar2->setPosition(0 - secPerDiv);

	/* Update Cursor Readouts */
	onVoltageCursor1Moved(d_hBar1->plotCoord().y());
	onVoltageCursor2Moved(d_hBar2->plotCoord().y());
	onTimeCursor1Moved(d_vBar1->plotCoord().x());
	onTimeCursor2Moved(d_vBar2->plotCoord().x());

	d_cursorMetricFormatter.setTwoDecimalMode(false);
	d_cursorTimeFormatter.setTwoDecimalMode(false);

	/* When a handle position changes the bar follows */
	connect(d_vCursorHandle1, SIGNAL(positionChanged(int)),
		d_hBar1, SLOT(setPixelPosition(int)));
	connect(d_vCursorHandle2, SIGNAL(positionChanged(int)),
		d_hBar2, SLOT(setPixelPosition(int)));
	connect(d_hCursorHandle1, SIGNAL(positionChanged(int)),
		d_vBar1, SLOT(setPixelPosition(int)));
	connect(d_hCursorHandle2, SIGNAL(positionChanged(int)),
		d_vBar2, SLOT(setPixelPosition(int)));

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
}

CapturePlot::~CapturePlot()
{
}

HorizBar *CapturePlot::levelTriggerA()
{
	return d_levelTriggerABar;
}

HorizBar *CapturePlot::levelTriggerB()
{
	return d_levelTriggerBBar;
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
}

void CapturePlot::onVbar2PixelPosChanged(int pos)
{
	d_hCursorHandle2->setPositionSilenty(pos);
}

void CapturePlot::onTimeCursor1Moved(double value)
{
	QString text;

	text = d_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor1Text(text);

	double diff = value - d_vBar2->plotCoord().x();
	text = d_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);

	if (diff !=0 )
		text = d_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
}

void CapturePlot::onTimeCursor2Moved(double value)
{
	QString text;

	text = d_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor2Text(text);

	double diff = d_vBar1->plotCoord().x() - value;
	text = d_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);

	if (diff !=0 )
		text = d_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
}

void CapturePlot::onVoltageCursor1Moved(double value)
{
	QString text;

	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor1Text(text);

	double diff = value - d_hBar2->plotCoord().y();
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(text);
}

void CapturePlot::onVoltageCursor2Moved(double value)
{
	QString text;

	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor2Text(text);

	double diff = d_hBar1->plotCoord().y() - value;
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(text);
}

QWidget * CapturePlot::topArea()
{
	return d_topWidget;
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

void CapturePlot::setMeasurementCursorsEnabled(bool en)
{
	if (d_measurementEnabled != en) {
		d_measurementEnabled = en;
		setVertCursorsEnabled(en);
		setHorizCursorsEnabled(en);
	}
}

bool CapturePlot::measurementCursorsEnabled()
{
	return d_measurementEnabled;
}

void CapturePlot::setVertCursorsEnabled(bool en)
{
	if (d_vertCursorsEnabled != en) {
		d_vertCursorsEnabled = en;
		d_vBar1->setVisible(en);
		d_vBar2->setVisible(en);
		d_hCursorHandle1->setVisible(en);
		d_hCursorHandle2->setVisible(en);
		d_cursorReadouts->setTimeReadoutVisible(en);
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
		d_cursorReadouts->setVoltageReadoutVisible(en);
	}
}

bool CapturePlot::horizCursorsEnabled()
{
	return d_horizCursorsEnabled;
}

void CapturePlot::onTimeTriggerHandlePosChanged(int pos)
{
	QwtScaleMap xMap = this->canvasMap(QwtAxisId(QwtPlot::xBottom, 0));
	double min = -(xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();
	double max = (xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();

	xMap.setScaleInterval(min, max);
	double time = xMap.invTransform(pos);
	emit timeTriggerValueChanged(-time);
}


void CapturePlot::onTriggerAHandleGrabbed(bool grabbed)
{
	if (grabbed)
		d_levelTriggerABar->setPen(d_trigAactiveLinePen);
	else
		d_levelTriggerABar->setPen(d_trigAinactiveLinePen);
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
