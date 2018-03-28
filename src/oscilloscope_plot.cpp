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

#include <QHBoxLayout>
#include <QLabel>

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
	d_timeTriggerMaxValue(1)
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
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());
	d_rightHandlesArea->setMinimumHeight(this->minimumHeight());

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
	d_sampleRateLabel->setStyleSheet("QLabel {"
		"color: #ffffff;"
		"}");

	// Trigger State
	d_triggerStateLabel = new QLabel(this);
	d_triggerStateLabel->setStyleSheet("QLabel {"
		"color: #ffffff;"
		"}");

	// Top area layout
	QHBoxLayout *topWidgetLayout = new QHBoxLayout(d_topWidget);
	topWidgetLayout->setContentsMargins(d_leftHandlesArea->minimumWidth(),
		0, d_rightHandlesArea->minimumWidth(), 5);

	topWidgetLayout->setSpacing(10);

	topWidgetLayout->insertWidget(0, d_timeBaseLabel, 0, Qt::AlignLeft |
		Qt::AlignBottom);
	topWidgetLayout->insertWidget(1, d_sampleRateLabel, 0, Qt::AlignLeft |
		Qt::AlignBottom);
	topWidgetLayout->insertWidget(2, d_triggerStateLabel, 0, Qt::AlignRight |
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

	/* Apply measurements for every new batch of data */
	connect(this, SIGNAL(newData()),
		SLOT(onNewDataReceived()));

	/* Add offset widgets for each new channel */
	connect(this, SIGNAL(channelAdded(int)),
		SLOT(onChannelAdded(int)));

	installEventFilter(this);
	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
	const int fmw = QFontMetrics(scaleWidget->font()).width("-XX.XX XX");
	scaleWidget->setMinBorderDist(fmw / 2, fmw / 2);
}

CapturePlot::~CapturePlot()
{
	removeEventFilter(this);
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
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

	value_v2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onVoltageCursor1Moved(double value)
{
	QString text;

	value *= d_displayScale;
	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor1Text(text);
	d_cursorReadoutsText.v1 = text;

	double diff = value - (d_hBar2->plotCoord().y() * d_displayScale) ;
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	value_h1 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onVoltageCursor2Moved(double value)
{
	QString text;

	value *= d_displayScale;
	text = d_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor2Text(text);
	d_cursorReadoutsText.v2 = text;

	double diff = (d_hBar1->plotCoord().y() * d_displayScale) - value;
	text = d_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	value_h2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
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

bool CapturePlot::enableAxisLabels(bool enabled)
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

bool CapturePlot::setActiveVertAxis(unsigned int axisIdx, bool selected)
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
	if (enabled) {
		d_bottomHandlesArea->setLeftPadding(50 + axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->width());
		QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
		const int fmw = QFontMetrics(scaleWidget->font()).width("-XX.XX XX");
		const int fmh = QFontMetrics(scaleWidget->font()).height();
		d_bottomHandlesArea->setRightPadding(50 + fmw/2);
		d_rightHandlesArea->setTopPadding(50 + 6);
		d_rightHandlesArea->setBottomPadding(50 + fmh);
	} else {
		if (d_bottomHandlesArea->leftPadding() != 50)
			d_bottomHandlesArea->setLeftPadding(50);
		if (d_bottomHandlesArea->rightPadding() != 50)
			d_bottomHandlesArea->setRightPadding(50);
		if (d_rightHandlesArea->topPadding() != 50)
			d_rightHandlesArea->setTopPadding(50);
		if (d_rightHandlesArea->bottomPadding() != 50)
			d_rightHandlesArea->setBottomPadding(50);
	}
}

bool CapturePlot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() && event->type() == QEvent::Resize) {
		updateHandleAreaPadding(d_labelsEnabled);
	}
	return QObject::eventFilter(object, event);
}

void CapturePlot::onChannelAdded(int chnIdx)
{
	setLeftVertAxesCount(chnIdx + 1);
	QColor chnColor = getLineColor(chnIdx);

	/* Channel offset widget */
	HorizBar *chOffsetBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(chOffsetBar);
	chOffsetBar->setCanLeavePlot(true);
	chOffsetBar->setVisible(false);
	chOffsetBar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, chnIdx));
	d_offsetBars.push_back(chOffsetBar);

	RoundedHandleV *chOffsetHdl = new RoundedHandleV(
				QPixmap(":/icons/handle_right_arrow.svg"),
				QPixmap(":/icons/handle_up_arrow.svg"),
				QPixmap(":/icons/handle_down_arrow.svg"),
				d_leftHandlesArea, true);
	chOffsetHdl->setRoundRectColor(chnColor);
	chOffsetHdl->setPen(QPen(chnColor, 2, Qt::SolidLine));
	chOffsetHdl->setVisible(true);
	d_offsetHandles.push_back(chOffsetHdl);

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

			Q_EMIT channelOffsetChanged(-offset);
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

				Q_EMIT channelOffsetChanged(offset);
			}
			if (offset < d_minOffsetValue){
				offset = d_minOffsetValue;
				this->setVertOffset(offset, chn_id);
				this->replot();

				Q_EMIT channelOffsetChanged(offset);
			}
	});

	/* Add Measure ojbect that handles all channel measurements */

	Measure *measure = nullptr;

	if (isReferenceWaveform(Curve(chnIdx))) {
		int idx = chnIdx - d_ydata.size();
		measure = new Measure(chnIdx, d_ref_ydata[idx],
			Curve(chnIdx)->data()->size());
	} else {
		int count = countReferenceWaveform(chnIdx);
		measure = new Measure(chnIdx, d_ydata[chnIdx - count],
			Curve(chnIdx)->data()->size());
	}
	measure->setAdcBitCount(12);
	d_measureObjs.push_back(measure);
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
	if (d_timeBaseLabel->text().contains("Zoom: ")) {
		d_timeBaseLabel->setText("Zoom: " + text + "/div");
	} else {
		d_timeBaseLabel->setText(text + "/div");
	}
}

void CapturePlot::setTimeBaseZoomed(bool zoomed)
{
	if (zoomed) {
		if (!d_timeBaseLabel->text().contains("Zoom: "))
			d_timeBaseLabel->setText("Zoom: " + d_timeBaseLabel->text());
	} else {
		QString text = d_timeBaseLabel->text();
		if (text.contains("Zoom: ")) {
			text = text.remove("Zoom: ");
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
		d_triggerStateLabel->setText("Waiting");
		break;
	case Triggered:
		d_triggerStateLabel->setText("Triggered");
		break;
	case Stop:
		d_triggerStateLabel->setText("Stop");
		break;
	case Auto:
		d_triggerStateLabel->setText("Auto");
		break;
	default:
		break;
	};
	d_triggerStateLabel->show();
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
	QString txtSampleRate = d_cursorMetricFormatter.format(sr, "Hz", 0);
	QString txtSamplingPeriod = d_cursorTimeFormatter.format(1 / sr, "", 0);
	QString text = QString("%1 Samples at ").arg(nsamples) + txtSampleRate +
		"/" + txtSamplingPeriod;
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
