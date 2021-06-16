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

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QThread>

#include <algorithm>
#include <scopy/gui/handles_area.hpp>
#include <scopy/gui/logicanalyzer/annotation_curve.hpp>
#include <scopy/gui/logicanalyzer/logic_data_curve.hpp>
#include <scopy/gui/oscilloscope_plot.hpp>
#include <scopy/gui/plot_line_handle.hpp>
#include <scopy/gui/symbol_controller.hpp>

#define ERROR_VALUE -10000000

using namespace scopy::gui;

/*
 * OscilloscopePlot class
 */
OscilloscopePlot::OscilloscopePlot(QWidget* parent, bool isdBgraph, unsigned int xNumDivs, unsigned int yNumDivs)
	: TimeDomainDisplayPlot(parent, isdBgraph, xNumDivs, yNumDivs)
{
	setYaxisUnit("V");

	setMinXaxisDivision(100E-9); // A minimum division of 100 nano second
	setMaxXaxisDivision(1E-3);   // A maximum division of 1 milli second - until adding decimation
	setMinYaxisDivision(1E-6);   // A minimum division of 1 micro Volts
	setMaxYaxisDivision(10.0);   // A maximum division of 10 Volts
}

OscilloscopePlot::~OscilloscopePlot() {}

/*
 * CapturePlot class
 */
CapturePlot::CapturePlot(QWidget* parent, bool isdBgraph, unsigned int xNumDivs, unsigned int yNumDivs)
	: OscilloscopePlot(parent, isdBgraph, xNumDivs, yNumDivs)
	, m_triggerAEnabled(false)
	, m_triggerBEnabled(false)
	, m_measurementsEnabled(false)
	, m_bufferSizeLabelVal(0)
	, m_sampleRateLabelVal(1.0)
	, m_labelsEnabled(false)
	, m_timeTriggerMinValue(-1)
	, m_timeTriggerMaxValue(1)
	, m_bonusWidth(0)
	, m_gatingEnabled(false)
	, m_conversion_function(nullptr)
	, m_startedGrouping(false)
	, m_xAxisInterval{0.0, 0.0}
	, m_currentHandleInitPx(30)
	, m_maxBufferError(nullptr)
{
	setMinimumHeight(200);
	setMinimumWidth(450);

	/* Initial colors scheme */
	m_trigAactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	m_trigAinactiveLinePen = QPen(QColor(175, 175, 175, 150), 2, Qt::DashLine);
	m_trigBactiveLinePen = QPen(QColor(255, 255, 255), 2, Qt::SolidLine);
	m_trigBinactiveLinePen = QPen(QColor(175, 175, 175), 2, Qt::DashLine);
	m_timeTriggerInactiveLinePen = QPen(QColor(74, 100, 255, 150), 2, Qt::DashLine);
	m_timeTriggerActiveLinePen = QPen(QColor(74, 100, 255), 2, Qt::SolidLine);
	/* End of: Initial colors scheme */

	setHorizUnitsPerDiv(1E-6);
	zoomBaseUpdate();

	/* Adjacent areas */
	m_topWidget = new QWidget(this);
	m_topGateHandlesArea = new GateHandlesArea(this->canvas());

	m_topWidget->setStyleSheet("QWidget {background-color: transparent}");
	m_topWidget->setMinimumHeight(50);
	m_topGateHandlesArea->setMinimumHeight(20);
	m_topGateHandlesArea->setLargestChildWidth(80);
	d_leftHandlesArea->setMinimumWidth(50);
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());

	m_topGateHandlesArea->hide();
	/* Add content to the top area of the plot */
	// Time Base
	m_timeBaseLabel = new QLabel(this);
	m_timeBaseLabel->setStyleSheet("QLabel {"
				       "color: #4a64ff;"
				       "font-weight: bold;"
				       "}");

	// Call to minimumSizeHint() is required. Otherwise font properties from
	// stylesheet will be ignored when calculating width using FontMetrics
	int width = m_timeBaseLabel->minimumSizeHint().width();
	QFontMetrics fm = m_timeBaseLabel->fontMetrics();
	width = fm.width("999.999 ms/div");
	m_timeBaseLabel->setMinimumWidth(width);

	// Sample Rate and Buffer Size
	m_sampleRateLabel = new QLabel("", this);

	// Trigger State
	m_triggerStateLabel = new QLabel(this);

	m_maxBufferError = new QLabel(this);
	m_maxBufferError->setStyleSheet("QLabel {"
					"color: #ff0000;"
					"}");

	m_maxBufferError->setWordWrap(true);

	// Top area layout
	QHBoxLayout* topWidgetLayout = new QHBoxLayout(m_topWidget);
	topWidgetLayout->setContentsMargins(d_leftHandlesArea->minimumWidth(), 0, d_rightHandlesArea->minimumWidth(),
					    5);

	topWidgetLayout->setSpacing(10);

	topWidgetLayout->insertWidget(0, m_timeBaseLabel, 0, Qt::AlignLeft | Qt::AlignBottom);
	topWidgetLayout->insertWidget(1, m_sampleRateLabel, 0, Qt::AlignLeft | Qt::AlignBottom);
	topWidgetLayout->insertWidget(2, m_maxBufferError, 0, Qt::AlignRight | Qt::AlignBottom);
	topWidgetLayout->insertWidget(3, m_triggerStateLabel, 0, Qt::AlignRight | Qt::AlignBottom);

	QSpacerItem* spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	topWidgetLayout->insertSpacerItem(2, spacerItem);

	m_topWidget->setLayout(topWidgetLayout);

	/* Time trigger widget */
	m_timeTriggerBar = new VertBar(this);
	d_symbolCtrl->attachSymbol(m_timeTriggerBar);
	m_timeTriggerBar->setPen(m_timeTriggerInactiveLinePen);
	m_timeTriggerBar->setCanLeavePlot(true);

	m_timeTriggerHandle = new FreePlotLineHandleH(QPixmap(":/icons/time_trigger_handle.svg"),
						      QPixmap(":/icons/time_trigger_left.svg"),
						      QPixmap(":/icons/time_trigger_right.svg"), d_bottomHandlesArea);
	m_timeTriggerHandle->setPen(m_timeTriggerActiveLinePen);

	connect(m_timeTriggerHandle, SIGNAL(grabbedChanged(bool)), SLOT(onTimeTriggerHandleGrabbed(bool)));

	connect(m_timeTriggerHandle, SIGNAL(positionChanged(int)), SLOT(onTimeTriggerHandlePosChanged(int)));

	/* When bar position changes due to plot resizes update the handle */
	connect(m_timeTriggerBar, &VertBar::pixelPositionChanged, [=](int pos) {
		updateHandleAreaPadding(m_labelsEnabled);
		m_timeTriggerHandle->setPositionSilenty(pos);
	});

	connect(m_timeTriggerHandle, &FreePlotLineHandleH::positionChanged, m_timeTriggerBar,
		&VertBar::setPixelPosition);

	connect(m_timeTriggerHandle, &RoundedHandleV::mouseReleased, [=]() {
		double pos = m_timeTriggerHandle->position();

		QwtScaleMap xMap = this->canvasMap(QwtAxisId(QwtPlot::xBottom, 0));
		double min = -(xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();
		double max = (xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();

		xMap.setScaleInterval(min, max);
		double time = xMap.invTransform(pos);

		if (time < m_timeTriggerMinValue) {
			m_timeTriggerBar->setPixelPosition(xMap.transform(m_timeTriggerMinValue));
		}
		if (time > m_timeTriggerMaxValue) {
			m_timeTriggerBar->setPixelPosition(xMap.transform(m_timeTriggerMaxValue));
		}
	});

	/* Level triggers widgets */
	// Trigger A
	m_levelTriggerABar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(m_levelTriggerABar);
	m_levelTriggerABar->setPen(m_timeTriggerInactiveLinePen);
	m_levelTriggerABar->setCanLeavePlot(true);

	m_levelTriggerAHandle = new FreePlotLineHandleV(QPixmap(":/icons/level_trigger_handle.svg"),
							QPixmap(":/icons/level_trigger_up.svg"),
							QPixmap(":/icons/level_trigger_down.svg"), d_rightHandlesArea);
	m_levelTriggerAHandle->setPen(m_timeTriggerActiveLinePen);

	m_levelTriggerABar->setVisible(false);
	m_levelTriggerAHandle->setVisible(false);

	/* When a handle position changes the bar follows */
	connect(m_levelTriggerAHandle, SIGNAL(positionChanged(int)), m_levelTriggerABar, SLOT(setPixelPosition(int)));
	/* When bar position changes due to plot resizes update the handle */
	connect(m_levelTriggerABar, &HorizBar::pixelPositionChanged,
		[=](int pos) { m_levelTriggerAHandle->setPositionSilenty(pos); });
	/* When handle is grabbed change bar drawing style */
	connect(m_levelTriggerAHandle, SIGNAL(grabbedChanged(bool)), SLOT(onTriggerAHandleGrabbed(bool)));

	// Trigger B
	m_levelTriggerBBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(m_levelTriggerBBar);
	m_levelTriggerBBar->setPen(m_trigBinactiveLinePen);
	m_levelTriggerBBar->setCanLeavePlot(true);

	m_levelTriggerBHandle = new FreePlotLineHandleV(QPixmap(":/icons/level_trigger_handle.svg"),
							QPixmap(":/icons/level_trigger_up.svg"),
							QPixmap(":/icons/level_trigger_down.svg"), d_rightHandlesArea);
	m_levelTriggerBHandle->setPen(m_trigBactiveLinePen);

	m_levelTriggerBBar->setVisible(false);
	m_levelTriggerBHandle->setVisible(false);

	/* When a handle position changes the bar follows */
	connect(m_levelTriggerBHandle, SIGNAL(positionChanged(int)), m_levelTriggerBBar, SLOT(setPixelPosition(int)));
	/* When bar position changes due to plot resizes update the handle */
	connect(m_levelTriggerBBar, &HorizBar::pixelPositionChanged,
		[=](int pos) { m_levelTriggerBHandle->setPositionSilenty(pos); });
	/* When handle is grabbed change bar drawing style */
	connect(m_levelTriggerBHandle, SIGNAL(grabbedChanged(bool)), SLOT(onTriggerBHandleGrabbed(bool)));

	/* Measurement gate cursors */
	m_hGatingHandle1 = new PlotGateHandle(QPixmap(":/icons/gate_handle.svg"), m_topGateHandlesArea);

	m_hGatingHandle2 = new PlotGateHandle(QPixmap(":/icons/gate_handle.svg"), m_topGateHandlesArea);

	m_hGatingHandle1->setCenterLeft(false);

	m_gateBar1 = new VertBar(this, true);
	m_gateBar2 = new VertBar(this, true);

	m_gateBar1->setVisible(false);
	m_gateBar2->setVisible(false);

	d_symbolCtrl->attachSymbol(m_gateBar1);
	d_symbolCtrl->attachSymbol(m_gateBar2);

	/* gate bars */
	QPen gatePen = QPen(QColor(255, 255, 255), 1, Qt::SolidLine);
	m_gateBar1->setPen(gatePen);
	m_gateBar2->setPen(gatePen);

	m_gateBar1->setVisible(false);
	m_gateBar2->setVisible(false);

	m_hGatingHandle1->hide();
	m_hGatingHandle2->hide();

	m_cursorMetricFormatter.setTwoDecimalMode(false);
	m_cursorTimeFormatter.setTwoDecimalMode(false);

	connect(m_hGatingHandle1, &PlotLineHandleH::positionChanged, [=](int value) {
		m_hGatingHandle2->setOtherCursorPosition(m_hGatingHandle1->position());
		/* make sure that the gate handles don't cross each other */
		if (m_hGatingHandle1->position() <= m_hGatingHandle2->position()) {

			m_gateBar1->setPixelPosition(value);

		} else {
			m_gateBar1->setPixelPosition(m_hGatingHandle2->position());
			m_hGatingHandle1->setPosition(m_hGatingHandle2->position());
		}
	});

	connect(m_hGatingHandle2, &PlotLineHandleH::positionChanged, [=](int value) {
		m_hGatingHandle1->setOtherCursorPosition(m_hGatingHandle2->position());
		/* make sure that the gate handles don't cross each other */
		if (m_hGatingHandle2->position() >= m_hGatingHandle1->position()) {
			m_gateBar2->setPixelPosition(value);
		} else {
			m_gateBar2->setPixelPosition(m_hGatingHandle1->position());
			m_hGatingHandle2->setPosition(m_hGatingHandle1->position());
		}
	});

	double secPerDiv = HorizUnitsPerDiv();
	m_gateBar1->setPosition(0 - 4 * secPerDiv);
	m_gateBar2->setPosition(0 + 4 * secPerDiv);

	/* initialise gate handle positions */
	m_hGatingHandle1->setOtherCursorPosition(m_hGatingHandle2->position());
	m_hGatingHandle2->setOtherCursorPosition(m_hGatingHandle1->position());

	connect(m_timeTriggerHandle, &FreePlotLineHandleH::reset, [=]() { Q_EMIT timeTriggerValueChanged(0); });
	connect(m_levelTriggerAHandle, &FreePlotLineHandleV::reset,
		[=]() { m_levelTriggerABar->setPlotCoord(QPointF(m_levelTriggerABar->plotCoord().x(), 0)); });

	connect(m_gateBar1, SIGNAL(pixelPositionChanged(int)), SLOT(onGateBar1PixelPosChanged(int)));
	connect(m_gateBar2, SIGNAL(pixelPositionChanged(int)), SLOT(onGateBar2PixelPosChanged(int)));

	connect(m_gateBar1, SIGNAL(positionChanged(double)), SLOT(onGateBar1Moved(double)));
	connect(m_gateBar2, SIGNAL(positionChanged(double)), SLOT(onGateBar2Moved(double)));

	/* Apply measurements for every new batch of data */
	connect(this, SIGNAL(newData()), SLOT(onNewDataReceived()));

	/* Add offset widgets for each new channel */
	connect(this, SIGNAL(channelAdded(int)), SLOT(onChannelAdded(int)));

	connect(this, &TimeDomainDisplayPlot::digitalPlotCurveAdded, this, &CapturePlot::onDigitalChannelAdded);

	installEventFilter(this);
	QwtScaleWidget* scaleWidget = axisWidget(QwtPlot::xBottom);
	const int fmw = QFontMetrics(scaleWidget->font()).width("-XXX.XXX XX");
	scaleWidget->setMinBorderDist(fmw / 2 + 30, fmw / 2 + 30);

	m_displayGraticule = false;

	m_graticule = new Graticule(this);
	connect(this, SIGNAL(canvasSizeChanged()), m_graticule, SLOT(onCanvasSizeChanged()));

	QBrush gateBrush = QBrush(QColor(0, 30, 150, 90));
	gateBrush.setStyle(Qt::SolidPattern);

	/* configure the measurement gates */
	m_leftGate = new QwtPlotShapeItem();
	m_leftGate->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
	m_leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_leftGateRect.setLeft(axisScaleDiv(xBottom).lowerBound());
	m_leftGateRect.setRight(m_gateBar1->plotCoord().x());
	m_leftGate->setRect(m_leftGateRect);
	m_leftGate->setBrush(gateBrush);

	m_rightGate = new QwtPlotShapeItem();
	m_rightGate->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
	m_rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_rightGateRect.setLeft(m_gateBar2->plotCoord().x());
	m_rightGateRect.setRight(axisScaleDiv(xBottom).upperBound());
	m_rightGate->setRect(m_rightGateRect);
	m_rightGate->setBrush(gateBrush);
}

CapturePlot::~CapturePlot()
{
	canvas()->removeEventFilter(d_cursorReadouts);
	removeEventFilter(this);
	canvas()->removeEventFilter(d_symbolCtrl);
	for (auto it = m_measureObjs.begin(); it != m_measureObjs.end(); ++it) {
		delete *it;
	}
	delete m_graticule;
	delete m_leftGate;
	delete m_rightGate;
}

QString CapturePlot::formatXValue(double value, int precision) const
{
	return m_cursorTimeFormatter.format(value, "", precision);
}

QString CapturePlot::formatYValue(double value, int precision) const
{
	return m_cursorMetricFormatter.format(value, "", precision);
}

CursorReadouts* CapturePlot::getCursorReadouts() const { return d_cursorReadouts; }

void CapturePlot::replot()
{

	TimeDomainDisplayPlot::replot();

	if (!d_bottomHandlesArea) {
		return;
	}

	const QwtInterval interval = axisInterval(QwtPlot::xBottom);
	if (interval.minValue() != m_xAxisInterval.first || interval.maxValue() != m_xAxisInterval.second) {

		d_bottomHandlesArea->repaint();

		m_xAxisInterval.first = interval.minValue();
		m_xAxisInterval.second = interval.maxValue();
	}
}

HorizBar* CapturePlot::levelTriggerA() { return m_levelTriggerABar; }

HorizBar* CapturePlot::levelTriggerB() { return m_levelTriggerBBar; }

void CapturePlot::enableTimeTrigger(bool enable)
{
	m_timeTriggerBar->setVisible(enable);
	m_timeTriggerHandle->setVisible(enable);
}

void CapturePlot::onVCursor1Moved(double value)
{
	QString text;
	text = m_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor1Text(text);
	d_cursorReadoutsText.t1 = text;

	double diff = value - d_vBar2->plotCoord().x();
	text = m_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	if (diff != 0)
		text = m_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
	d_cursorReadoutsText.freq = text;
	if (d_trackMode) {
		onHCursor1Moved(getHorizontalCursorIntersection(d_vBar1->plotCoord().x()));
	}

	m_valueV1 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onVCursor2Moved(double value)
{
	QString text;
	text = m_cursorTimeFormatter.format(value, "", 3);
	d_cursorReadouts->setTimeCursor2Text(text);
	d_cursorReadoutsText.t2 = text;

	double diff = d_vBar1->plotCoord().x() - value;
	text = m_cursorTimeFormatter.format(diff, "", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.tDelta = text;

	if (diff != 0)
		text = m_cursorMetricFormatter.format(1 / diff, "Hz", 3);
	else
		text = "Infinity";
	d_cursorReadouts->setFreqDeltaText(text);
	d_cursorReadoutsText.freq = text;
	if (d_trackMode) {
		onHCursor2Moved(getHorizontalCursorIntersection(d_vBar2->plotCoord().x()));
	}

	m_valueV2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onHCursor1Moved(double value)
{
	QString text;

	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = m_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor1Text(error ? "-" : text);
	d_cursorReadoutsText.v1 = error ? "-" : text;

	double valueCursor2;
	if (d_trackMode) {
		valueCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());
	} else {
		valueCursor2 = d_hBar2->plotCoord().y();
	}

	double diff = value - (valueCursor2 * d_displayScale);
	text = m_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	m_valueH1 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onHCursor2Moved(double value)
{
	QString text;

	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = m_cursorMetricFormatter.format(value, "V", 3);
	d_cursorReadouts->setVoltageCursor2Text(error ? "-" : text);
	d_cursorReadoutsText.v2 = error ? "-" : text;

	double valueCursor1;
	if (d_trackMode) {
		valueCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	} else {
		valueCursor1 = d_hBar1->plotCoord().y();
	}

	double diff = (valueCursor1 * d_displayScale) - value;
	text = m_cursorMetricFormatter.format(diff, "V", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.vDelta = error ? "-" : text;

	m_valueH2 = value;
	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void CapturePlot::onGateBar1PixelPosChanged(int pos)
{
	m_hGatingHandle1->setPositionSilenty(pos);
	m_hGatingHandle2->setOtherCursorPosition(m_hGatingHandle1->position());
}

void CapturePlot::onGateBar2PixelPosChanged(int pos)
{
	m_hGatingHandle2->setPositionSilenty(pos);
	m_hGatingHandle1->setOtherCursorPosition(m_hGatingHandle2->position());
}

void CapturePlot::onGateBar1Moved(double value)
{

	if (d_selected_channel < 0) {
		return;
	}

	// update gate handle
	m_leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_leftGateRect.setLeft(axisScaleDiv(xBottom).lowerBound());
	m_leftGateRect.setRight(value);
	m_leftGate->setRect(m_leftGateRect);

	int n = Curve(d_selected_channel)->data()->size();

	double maxTime = 0;
	double minTime = 0;

	if (n == 0) {
		maxTime = axisScaleDiv(xBottom).upperBound();
		minTime = axisScaleDiv(xBottom).lowerBound();
	} else {
		maxTime = Curve(d_selected_channel)->data()->sample(n - 1).x();
		minTime = Curve(d_selected_channel)->data()->sample(0).x();
	}

	// data index to start measurement
	int currentIndex = (value - minTime) / (maxTime - minTime) * n;

	for (int i = 0; i < m_measureObjs.size(); i++) {
		Measure* measure = m_measureObjs[i];
		measure->setStartIndex(currentIndex);
	}

	m_valueGateLeft = value;
	// find the percentage of the gate in relation with plot width
	double width = (value - axisScaleDiv(xBottom).lowerBound()) /
		(axisScaleDiv(xBottom).upperBound() - axisScaleDiv(xBottom).lowerBound());
	Q_EMIT leftGateChanged(width);
	m_hGatingHandle1->setTimeValue(m_gateBar1->plotCoord().x());

	replot();
}

void CapturePlot::onGateBar2Moved(double value)
{

	if (d_selected_channel < 0) {
		return;
	}

	// update gate handle
	m_rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_rightGateRect.setLeft(value);
	m_rightGateRect.setRight(axisScaleDiv(xBottom).upperBound());
	m_rightGate->setRect(m_rightGateRect);

	int n = Curve(d_selected_channel)->data()->size();

	double maxTime = 0;
	double minTime = 0;

	if (n == 0) {
		maxTime = axisScaleDiv(xBottom).upperBound();
		minTime = axisScaleDiv(xBottom).lowerBound();
	} else {
		maxTime = Curve(d_selected_channel)->data()->sample(n - 1).x();
		minTime = Curve(d_selected_channel)->data()->sample(0).x();
	}

	// data index to end measurement
	int currentIndex = (value - minTime) / (maxTime - minTime) * n;

	for (int i = 0; i < m_measureObjs.size(); i++) {
		Measure* measure = m_measureObjs[i];
		measure->setEndIndex(currentIndex);
	}

	m_valueGateRight = value;
	// find the percentage of the gate in relation with plot width
	double width = (axisScaleDiv(xBottom).upperBound() - value) /
		(axisScaleDiv(xBottom).upperBound() - axisScaleDiv(xBottom).lowerBound());
	Q_EMIT rightGateChanged(width);
	m_hGatingHandle2->setTimeValue(m_gateBar2->plotCoord().x());

	replot();
}

QWidget* CapturePlot::topArea() { return m_topWidget; }

QWidget* CapturePlot::topHandlesArea()
{
	/* handle area for gate cursors */
	return m_topGateHandlesArea;
}

void CapturePlot::setBonusWidthForHistogram(int width) { m_bonusWidth = width; }

void CapturePlot::setTriggerAEnabled(bool en)
{
	if (m_triggerAEnabled != en) {
		m_triggerAEnabled = en;
		m_levelTriggerABar->setVisible(en);
		m_levelTriggerAHandle->setVisible(en);
	}
}

bool CapturePlot::triggerAEnabled() { return m_triggerAEnabled; }

void CapturePlot::setTriggerBEnabled(bool en)
{
	if (m_triggerBEnabled != en) {
		m_triggerBEnabled = en;
		m_levelTriggerBBar->setVisible(en);
		m_levelTriggerBHandle->setVisible(en);
	}
}

bool CapturePlot::triggerBEnabled() { return m_triggerBEnabled; }

void CapturePlot::setSelectedChannel(int id)
{
	if (d_selected_channel != id) {
		d_selected_channel = id;
	}
}

int CapturePlot::selectedChannel() { return d_selected_channel; }

void CapturePlot::setMeasuremensEnabled(bool en) { m_measurementsEnabled = en; }

bool CapturePlot::measurementsEnabled() { return m_measurementsEnabled; }

void CapturePlot::onTimeTriggerHandlePosChanged(int pos)
{
	QwtScaleMap xMap = this->canvasMap(QwtAxisId(QwtPlot::xBottom, 0));
	double min = -(xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();
	double max = (xAxisNumDiv() / 2.0) * HorizUnitsPerDiv();

	xMap.setScaleInterval(min, max);
	double time = xMap.invTransform(pos);
	Q_EMIT timeTriggerValueChanged(-time);
}

void CapturePlot::onTimeTriggerHandleGrabbed(bool grabbed)
{
	if (grabbed)
		m_timeTriggerBar->setPen(m_timeTriggerActiveLinePen);
	else
		m_timeTriggerBar->setPen(m_timeTriggerInactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::onTriggerAHandleGrabbed(bool grabbed)
{
	if (grabbed)
		m_levelTriggerABar->setPen(m_timeTriggerActiveLinePen);
	else
		m_levelTriggerABar->setPen(m_timeTriggerInactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::onTriggerBHandleGrabbed(bool grabbed)
{
	if (grabbed)
		m_levelTriggerBBar->setPen(m_trigBactiveLinePen);
	else
		m_levelTriggerBBar->setPen(m_trigBinactiveLinePen);
	d_symbolCtrl->updateOverlay();
}

void CapturePlot::showEvent(QShowEvent* event)
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
	QwtText text(m_timeBaseLabel->text() + " " + m_sampleRateLabel->text());
	text.setColor(QColor(0, 0, 0));
	detailsMarker.setLabel(text);
	replot();

	DisplayPlot::printWithNoBackground(toolName, editScaleDraw);
}

int CapturePlot::getAnalogChannels() const { return d_ydata.size() + d_ref_ydata.size(); }

Measure* CapturePlot::measureOfChannel(int chnIdx) const
{
	Measure* measure = nullptr;

	auto it = std::find_if(m_measureObjs.begin(), m_measureObjs.end(),
			       [&](Measure* m) { return m->channel() == chnIdx; });
	if (it != m_measureObjs.end())
		measure = *it;

	return measure;
}

void CapturePlot::bringCurveToFront(unsigned int curveIdx)
{
	for (auto& item : m_offsetHandles) {
		if (item->pen().color() == getLineColor(curveIdx))
			item->raise();
	}

	DisplayPlot::bringCurveToFront(curveIdx);
}

void CapturePlot::enableLabels(bool enabled)
{
	m_labelsEnabled = enabled;
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
			setAxisVisible(QwtAxisId(QwtPlot::yLeft, i), enabled);
		}
	}
}

void CapturePlot::setDisplayScale(double value)
{
	DisplayPlot::setDisplayScale(value);
	onHCursor1Moved(d_hBar1->plotCoord().y());
	onHCursor2Moved(d_hBar2->plotCoord().y());
}

void CapturePlot::setTimeTriggerInterval(double min, double max)
{
	m_timeTriggerMinValue = min;
	m_timeTriggerMaxValue = max;
}

bool CapturePlot::labelsEnabled() { return m_labelsEnabled; }

void CapturePlot::setGraticuleEnabled(bool enabled)
{
	m_displayGraticule = enabled;

	if (!m_displayGraticule) {
		for (QwtPlotScaleItem* scale : qAsConst(scaleItems)) {
			scale->attach(this);
		}
		m_graticule->enableGraticule(m_displayGraticule);
	} else {
		for (QwtPlotScaleItem* scale : qAsConst(scaleItems)) {
			scale->detach();
		}
		m_graticule->enableGraticule(m_displayGraticule);
	}

	replot();
}

void CapturePlot::setGatingEnabled(bool enabled)
{
	if (m_gatingEnabled != enabled) {
		m_gatingEnabled = enabled;
		m_gateBar1->setVisible(enabled);
		m_gateBar2->setVisible(enabled);
		m_hGatingHandle1->setVisible(enabled);
		m_hGatingHandle2->setVisible(enabled);
		updateHandleAreaPadding(m_labelsEnabled);

		if (enabled) {
			m_leftGate->attach(this);
			m_rightGate->attach(this);
			m_topGateHandlesArea->show();
			// update handle
			onGateBar1Moved(m_leftGateRect.right());
			onGateBar2Moved(m_rightGateRect.left());
		} else {
			m_leftGate->detach();
			m_rightGate->detach();
			m_topGateHandlesArea->hide();
		}
		for (int i = 0; i < m_measureObjs.size(); i++) {
			Measure* measure = m_measureObjs[i];
			measure->setGatingEnabled(enabled);
		}

		m_gateBar1->triggerMove();
		m_gateBar2->triggerMove();

		replot();
	}
}

void CapturePlot::setActiveVertAxis(unsigned int axisIdx, bool selected)
{
	DisplayPlot::setActiveVertAxis(axisIdx, selected);
	updateHandleAreaPadding(m_labelsEnabled);
	if (m_labelsEnabled) {
		enableAxis(QwtPlot::xBottom, true);
	}
}

void CapturePlot::showYAxisWidget(unsigned int axisIdx, bool en)
{
	if (!m_labelsEnabled)
		return;

	setAxisVisible(QwtAxisId(QwtPlot::yLeft, axisIdx), en);

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
		d_bottomHandlesArea->setLeftPadding(50 +
						    axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->width());
		m_topGateHandlesArea->setLeftPadding(90 +
						     axisWidget(QwtAxisId(QwtPlot::yLeft, d_activeVertAxis))->width());
		QwtScaleWidget* scaleWidget = axisWidget(QwtPlot::xBottom);
		const int fmw = QFontMetrics(scaleWidget->font()).width("-XX.XX XX");
		const int fmh = QFontMetrics(scaleWidget->font()).height();
		d_bottomHandlesArea->setRightPadding(50 + fmw / 2 + m_bonusWidth);
		m_topGateHandlesArea->setRightPadding(50 + fmw / 2 + m_bonusWidth);
		d_rightHandlesArea->setTopPadding(50 + 6);
		d_rightHandlesArea->setBottomPadding(50 + fmh);
		QMargins margins = m_topWidget->layout()->contentsMargins();
		margins.setLeft(d_leftHandlesArea->minimumWidth() + 100);
		m_topWidget->layout()->setContentsMargins(margins);
	} else {
		if (m_topGateHandlesArea->leftPadding() != 90)
			m_topGateHandlesArea->setLeftPadding(90);
		if (m_topGateHandlesArea->rightPadding() != 90)
			m_topGateHandlesArea->setRightPadding(90);
		if (d_bottomHandlesArea->leftPadding() != 50 + xAxisBonusWidth)
			d_bottomHandlesArea->setLeftPadding(50 + xAxisBonusWidth);
		if (d_bottomHandlesArea->rightPadding() != 50 + m_bonusWidth + xAxisBonusWidth)
			d_bottomHandlesArea->setRightPadding(50 + m_bonusWidth + xAxisBonusWidth);
		if (d_rightHandlesArea->topPadding() != 50)
			d_rightHandlesArea->setTopPadding(50);
		if (d_rightHandlesArea->bottomPadding() != 50)
			d_rightHandlesArea->setBottomPadding(50);

		int topPadding = m_gatingEnabled ? m_topGateHandlesArea->height() : 0;
		d_leftHandlesArea->setTopPadding(50 + topPadding);
		d_rightHandlesArea->setTopPadding(50 + topPadding);

		QMargins margins = m_topWidget->layout()->contentsMargins();
		margins.setLeft(d_leftHandlesArea->minimumWidth());
		m_topWidget->layout()->setContentsMargins(margins);
	}

	// update handle position to avoid cursors getting out of the plot bounds when changing the padding;
	d_hCursorHandle1->updatePosition();
	d_hCursorHandle2->updatePosition();

	d_vCursorHandle1->updatePosition();
	d_vCursorHandle2->updatePosition();
}

void CapturePlot::updateGateMargins()
{
	/* update the size of the gates */
	m_leftGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_leftGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_leftGate->setRect(m_leftGateRect);

	m_rightGateRect.setTop(axisScaleDiv(yRight).upperBound());
	m_rightGateRect.setBottom(axisScaleDiv(yRight).lowerBound());
	m_rightGate->setRect(m_rightGateRect);

	replot();
}

bool CapturePlot::eventFilter(QObject* object, QEvent* event)
{
	if (object == canvas() && event->type() == QEvent::Resize) {
		updateHandleAreaPadding(m_labelsEnabled);

		// force cursor handles to emit position changed
		// when the plot canvas is being resized
		d_hCursorHandle1->triggerMove();
		d_hCursorHandle2->triggerMove();
		d_vCursorHandle1->triggerMove();
		d_vCursorHandle2->triggerMove();
		m_timeTriggerHandle->triggerMove();
		m_levelTriggerAHandle->triggerMove();
		m_levelTriggerBHandle->triggerMove();

		/* update the size of the gates when the plot canvas is resized */
		updateGateMargins();

		for (int i = 0; i < m_offsetHandles.size(); ++i) {
			m_offsetHandles[i]->triggerMove();
		}

		Q_EMIT canvasSizeChanged();
	}
	return QObject::eventFilter(object, event);
}

QString CapturePlot::getChannelName(int chIdx) const
{
	if (chIdx >= m_offsetHandles.size() || chIdx < 0) {
		return "";
	}

	return m_offsetHandles.at(chIdx)->getName();
}

void CapturePlot::setChannelName(const QString& name, int chIdx)
{
	if (chIdx >= m_offsetHandles.size() || chIdx < 0) {
		return;
	}

	m_offsetHandles.at(chIdx)->setName(name);
}

void CapturePlot::removeDigitalPlotCurve(QwtPlotCurve* curve)
{
	for (int i = 0; i < m_offsetHandles.size(); ++i) {
		if (curve == getDigitalPlotCurve(i)) {
			removeOffsetWidgets(d_ydata.size() + d_ref_ydata.size() + i);
			removeLeftVertAxis(d_ydata.size() + d_ref_ydata.size() + i);
			break;
		}
	}

	TimeDomainDisplayPlot::removeDigitalPlotCurve(curve);

	replot();
}

void CapturePlot::setOffsetHandleVisible(int chIdx, bool visible) { m_offsetHandles.at(chIdx)->setVisible(visible); }

void CapturePlot::addToGroup(int currentGroup, int toAdd)
{
	const bool selected = m_offsetHandles.at(currentGroup)->isSelected();
	m_offsetHandles.at(currentGroup)->selected(true);
	m_offsetHandles.at(currentGroup)->setSelected(true);

	beginGroupSelection();

	m_offsetHandles.at(toAdd)->selected(true);
	endGroupSelection(true);

	m_offsetHandles.at(currentGroup)->setSelected(selected);
	m_offsetHandles.at(currentGroup)->selected(selected);
}

void CapturePlot::onDigitalChannelAdded(int chnIdx)
{
	setLeftVertAxesCount(d_ydata.size() + d_ref_ydata.size() + chnIdx + 1);
	setAxisScale(QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx), -5, 5);
	replot();

	QColor chnColor;
	const int h = (55 * chnIdx) % 360;
	const int s = 180;
	const int v = 170;
	chnColor.setHsl(h, s, v);

	QwtPlotCurve* curve = getDigitalPlotCurve(chnIdx);
	GenericLogicPlotCurve* logicCurve = dynamic_cast<GenericLogicPlotCurve*>(curve);

	curve->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx));

	/* Channel offset widget */
	HorizBar* chOffsetBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(chOffsetBar);
	chOffsetBar->setCanLeavePlot(true);
	chOffsetBar->setVisible(false);
	chOffsetBar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + chnIdx));
	m_offsetBars.push_back(chOffsetBar);

	RoundedHandleV* chOffsetHdl =
		new RoundedHandleV(QPixmap(":/icons/handle_right_arrow.svg"), QPixmap(":/icons/handle_up_arrow.svg"),
				   QPixmap(":/icons/handle_down_arrow.svg"), d_leftHandlesArea, true,
				   logicCurve->getName(), true); // TODO: add name
	chOffsetHdl->setRoundRectColor(chnColor);
	chOffsetHdl->setPen(QPen(chnColor, 2, Qt::SolidLine));
	chOffsetHdl->setVisible(true);
	m_offsetHandles.push_back(chOffsetHdl);

	connect(logicCurve, &GenericLogicPlotCurve::nameChanged,
		[=](const QString& name) { chOffsetHdl->setName(name); });

	connect(logicCurve, &GenericLogicPlotCurve::pixelOffsetChanged,
		[=](double offset) { chOffsetBar->setPosition(offset); });

	connect(chOffsetHdl, &RoundedHandleV::selected, [=](bool selected) {
		Q_EMIT channelSelected(m_offsetHandles.indexOf(chOffsetHdl), selected);

		if (!selected && !m_startedGrouping) {
			return;
		}

		if (m_startedGrouping) {
			chOffsetHdl->setSelected(selected);
			if (selected) {
				m_groupHandles.back().push_back(chOffsetHdl);
			} else {
				m_groupHandles.back().removeOne(chOffsetHdl);
			}
			return;
		}

		for (auto& hdl : m_offsetHandles) {
			if (hdl == chOffsetHdl) {
				continue;
			}

			hdl->setSelected(false);
		}
	});

	/* When bar position changes due to plot resizes update the handle */
	connect(chOffsetBar, &HorizBar::pixelPositionChanged, [=](int pos) { chOffsetHdl->setPositionSilenty(pos); });

	connect(chOffsetHdl, &RoundedHandleV::positionChanged, [=](int pos) {
		int chn_id = m_offsetHandles.indexOf(chOffsetHdl);
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

		QwtPlotCurve* curve = getDigitalPlotCurve(chnIdx);
		GenericLogicPlotCurve* logicCurve = dynamic_cast<GenericLogicPlotCurve*>(curve);

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

	chOffsetHdl->setPosition(m_currentHandleInitPx);
	m_currentHandleInitPx += 20;
}

void CapturePlot::setChannelSelectable(int chnIdx, bool selectable)
{
	m_offsetHandles.at(chnIdx)->setSelectable(selectable);
}

void CapturePlot::beginGroupSelection()
{
	if (m_startedGrouping) {
		qDebug() << "\"beginGroupSelection\" already called. Consider terminating current group"
			    "creation using \"endGroupSelection\"!";
	}

	m_startedGrouping = true;

	m_groupHandles.push_back(QList<RoundedHandleV*>());
	for (auto& hdl : m_offsetHandles) {
		if (hdl->isSelected()) {
			m_groupHandles.back().push_back(hdl);
		}
	}
}

bool CapturePlot::endGroupSelection(bool moveAnnotationCurvesLast)
{
	if (!m_startedGrouping) {
		qDebug() << "\"endGroupSelection\" call not paired with \"beginGroupSelection\"!";
		return false;
	}

	m_startedGrouping = false;

	if (m_groupHandles.back().size() < 2) {
		m_groupHandles.pop_back();
		return false;
	}

	for (auto& grp : m_groupHandles) {
		for (RoundedHandleV* hdl : grp) {
			disconnect(hdl, &RoundedHandleV::positionChanged, this,
				   &CapturePlot::handleInGroupChangedPosition);
		}
	}

	// merge new group if selected channels already have a group
	QList<RoundedHandleV*> group = m_groupHandles.takeLast();
	QList<RoundedHandleV*> updatedGroup;
	for (RoundedHandleV* hdl : qAsConst(group)) {
		auto hdlGroup =
			std::find_if(m_groupHandles.begin(), m_groupHandles.end(),
				     [&hdl](const QList<RoundedHandleV*>& group) { return group.contains(hdl); });

		if (hdlGroup == m_groupHandles.end()) {
			if (!updatedGroup.contains(hdl)) {
				updatedGroup.push_back(hdl);
			}
			continue;
		}

		auto hdlGroupContainer = *hdlGroup;
		for (const auto& grpHdl : qAsConst(hdlGroupContainer)) {
			if (!updatedGroup.contains(grpHdl)) {
				updatedGroup.push_back(grpHdl);
			}
		}

		m_groupHandles.removeOne(*hdlGroup);
	}
	group = updatedGroup;
	//
	if (moveAnnotationCurvesLast) {
		std::sort(group.begin(), group.end(), [=](RoundedHandleV* a, RoundedHandleV* b) {
			return m_offsetHandles.indexOf(a) < m_offsetHandles.indexOf(b);
		});
	}

	m_groupHandles.push_back(group);

	auto getTraceHeightInPixelsForHandle = [=](RoundedHandleV* handle, double& bonusHeight) {
		const int chIdx = m_offsetHandles.indexOf(handle);
		QwtPlotCurve* curve = getDigitalPlotCurve(chIdx);
		GenericLogicPlotCurve* logicCurve = dynamic_cast<GenericLogicPlotCurve*>(curve);
		AnnotationCurve* annCurve = dynamic_cast<AnnotationCurve*>(logicCurve);
		if (annCurve) {
			bonusHeight = annCurve->getTraceHeight() * annCurve->getVisibleRows();
			return static_cast<double>(handle->size().height());
		}
		bonusHeight = 0.0;
		return logicCurve->getTraceHeight();
	};

	const bool newGroup = (m_groupHandles.size() != m_groupMarkers.size());

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

	for (auto& grp : m_groupHandles) {
		for (RoundedHandleV* hdl : grp) {
			connect(hdl, &RoundedHandleV::positionChanged, this,
				&CapturePlot::handleInGroupChangedPosition);
			hdl->setSelected(false);
		}
	}

	group.first()->setSelected(true);
	group.first()->selected(true);

	for (QwtPlotZoneItem* groupMarker : qAsConst(m_groupMarkers)) {
		groupMarker->detach();
		delete groupMarker;
	}

	m_groupMarkers.clear();

	for (const auto& group : qAsConst(m_groupHandles)) {
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

		QwtPlotZoneItem* groupMarker = new QwtPlotZoneItem();
		m_groupMarkers.push_back(groupMarker);
		groupMarker->setAxes(QwtPlot::xBottom, QwtAxisId(QwtPlot::yLeft, 0));
		groupMarker->setPen(QColor(74, 100, 255, 30), 2.0);
		groupMarker->setBrush(QBrush(QColor(74, 100, 255, 10)));
		groupMarker->setInterval(y2, y1);
		groupMarker->setOrientation(Qt::Horizontal);
		groupMarker->attach(this);
	}

	if (!newGroup) {
		for (const auto& group : qAsConst(m_groupHandles)) {
			group.first()->triggerMove();
		}
	}

	return true;
}

QVector<int> CapturePlot::getGroupOfChannel(int chnIdx)
{
	QVector<int> groupIdxList;

	if (chnIdx < 0 || chnIdx >= m_offsetHandles.size()) {
		return groupIdxList; // empty
	}

	auto hdlGroup =
		std::find_if(m_groupHandles.begin(), m_groupHandles.end(), [=](const QList<RoundedHandleV*>& group) {
			return group.contains(m_offsetHandles[chnIdx]);
		});

	// if no group return
	if (hdlGroup == m_groupHandles.end()) {
		return groupIdxList;
	}

	auto hdlGroupContainer = *hdlGroup;
	for (const auto& hdl : qAsConst(hdlGroupContainer)) {
		groupIdxList.push_back(m_offsetHandles.indexOf(hdl));
	}

	return groupIdxList;
}

QVector<QVector<int>> CapturePlot::getAllGroups()
{
	QVector<QVector<int>> allGroups;
	for (int i = 0; i < m_groupHandles.size(); ++i) {
		QVector<int> group;
		for (int j = 0; j < m_groupHandles[i].size(); ++j) {
			int ch = m_offsetHandles.indexOf(m_groupHandles[i][j]);
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

void CapturePlot::removeFromGroup(int chnIdx, int removedChnIdx, bool& didGroupVanish)
{
	auto hdlGroup =
		std::find_if(m_groupHandles.begin(), m_groupHandles.end(), [=](const QList<RoundedHandleV*>& group) {
			return group.contains(m_offsetHandles[chnIdx]);
		});

	if (hdlGroup == m_groupHandles.end()) {
		return;
	}

	const int positionOfFirstHandleInGroup = hdlGroup->at(0)->position();
	const int positionOfLastHandleInGroup = hdlGroup->back()->position();

	disconnect(hdlGroup->at(removedChnIdx), &RoundedHandleV::positionChanged, this,
		   &CapturePlot::handleInGroupChangedPosition);

	auto removedObj = hdlGroup->takeAt(removedChnIdx);

	if (hdlGroup->size() < 2) {
		didGroupVanish = true;
		disconnect(hdlGroup->first(), &RoundedHandleV::positionChanged, this,
			   &CapturePlot::handleInGroupChangedPosition);
		const int indexOfCurrentGroup = m_groupHandles.indexOf(*hdlGroup);
		m_groupHandles.removeOne(*hdlGroup);
		auto marker = m_groupMarkers.takeAt(indexOfCurrentGroup);
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
	auto hdlGroup =
		std::find_if(m_groupHandles.begin(), m_groupHandles.end(), [=](const QList<RoundedHandleV*>& group) {
			return group.contains(m_offsetHandles[chnIdx]);
		});

	if (hdlGroup == m_groupHandles.end()) {
		return;
	}

	const int positionOfFirstHandleInGroup = hdlGroup->at(0)->position();

	auto item = hdlGroup->takeAt(from);
	hdlGroup->insert(to, item);

	hdlGroup->at(0)->setPosition(positionOfFirstHandleInGroup);
	hdlGroup->at(0)->triggerMove();
}

void CapturePlot::setGroups(const QVector<QVector<int>>& groups)
{
	auto selectedHandleIt = std::find_if(m_offsetHandles.begin(), m_offsetHandles.end(),
					     [](RoundedHandleV* handle) { return handle->isSelected(); });

	if (selectedHandleIt != m_offsetHandles.end()) {
		(*selectedHandleIt)->setSelected(false);
		(*selectedHandleIt)->selected(false);
	}

	for (const auto& grp : groups) {
		if (grp.size() < 2) {
			continue;
		}
		beginGroupSelection();
		for (const auto& hdl : grp) {
			m_groupHandles.back().push_back(m_offsetHandles.at(hdl));
		}
		endGroupSelection();

		m_groupHandles.back().front()->setSelected(false);
		m_groupHandles.back().front()->selected(false);
		m_groupHandles.back().front()->setPosition(m_groupHandles.back().front()->position());
	}

	if (selectedHandleIt != m_offsetHandles.end()) {
		(*selectedHandleIt)->setSelected(true);
		(*selectedHandleIt)->selected(true);
	}

	replot();
}

void CapturePlot::handleInGroupChangedPosition(int position)
{
	RoundedHandleV* hdl = dynamic_cast<RoundedHandleV*>(QObject::sender());

	// is the sender a RoundedHandleV?
	if (!hdl) {
		qDebug() << "Invalid sender!";
	}

	// find the group of this handle
	auto hdlGroup = std::find_if(m_groupHandles.begin(), m_groupHandles.end(),
				     [&hdl](const QList<RoundedHandleV*>& group) { return group.contains(hdl); });

	// if no group return
	if (hdlGroup == m_groupHandles.end()) {
		return;
	}

	// index of handle in the group
	const int index = hdlGroup->indexOf(hdl);
	const int groupIndex = m_groupHandles.indexOf(*hdlGroup);

	auto getTraceHeightInPixelsForHandle = [=](RoundedHandleV* handle, double& bonusHeight) {
		const int chIdx = m_offsetHandles.indexOf(handle);
		QwtPlotCurve* curve = getDigitalPlotCurve(chIdx);
		GenericLogicPlotCurve* logicCurve = dynamic_cast<GenericLogicPlotCurve*>(curve);
		AnnotationCurve* annCurve = dynamic_cast<AnnotationCurve*>(logicCurve);
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
		disconnect(hdlGroup->at(i), &RoundedHandleV::positionChanged, this,
			   &CapturePlot::handleInGroupChangedPosition);
		hdlGroup->at(i)->setPosition(currentPos);
		int temp = getTraceHeightInPixelsForHandle(hdlGroup->at(i), bonusHeight);
		if (bonusHeight != 0.0) {
			hdlGroup->at(i)->setPosition(currentPos - bonusHeight);
			currentPos -= bonusHeight;
			bonusHeight = 0.0;
		}
		currentPos -= temp;
		currentPos -= 5;
		connect(hdlGroup->at(i), &RoundedHandleV::positionChanged, this,
			&CapturePlot::handleInGroupChangedPosition);
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
		disconnect(hdlGroup->at(i), &RoundedHandleV::positionChanged, this,
			   &CapturePlot::handleInGroupChangedPosition);
		hdlGroup->at(i)->setPosition(currentPos);
		connect(hdlGroup->at(i), &RoundedHandleV::positionChanged, this,
			&CapturePlot::handleInGroupChangedPosition);
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

		m_groupMarkers.at(groupIndex)->setInterval(y2, y1);
	}

	replot();
}

void CapturePlot::pushBackNewOffsetWidgets(RoundedHandleV* chOffsetHdl, HorizBar* chOffsetBar)
{
	const int indexOfNewChannel = (d_ydata.size() + d_ref_ydata.size()) - 1;
	m_offsetBars.insert(indexOfNewChannel, chOffsetBar);
	m_offsetHandles.insert(indexOfNewChannel, chOffsetHdl);

	for (int i = 0; i < m_offsetBars.size(); ++i) {
		m_offsetBars[i]->setMobileAxis(QwtAxisId(QwtPlot::yLeft, i));
	}

	for (int i = 0; i < d_logic_curves.size(); ++i) {
		d_logic_curves[i]->setAxes(QwtPlot::xBottom,
					  QwtAxisId(QwtPlot::yLeft, d_ydata.size() + d_ref_ydata.size() + i));
	}
}

void CapturePlot::onChannelAdded(int chnIdx)
{
	setLeftVertAxesCount(m_offsetHandles.size() + 1);
	QColor chnColor = getLineColor(chnIdx);

	/* Channel offset widget */
	HorizBar* chOffsetBar = new HorizBar(this);
	d_symbolCtrl->attachSymbol(chOffsetBar);
	chOffsetBar->setCanLeavePlot(true);
	chOffsetBar->setVisible(false);
	chOffsetBar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, chnIdx));

	RoundedHandleV* chOffsetHdl =
		new RoundedHandleV(QPixmap(":/icons/handle_right_arrow.svg"), QPixmap(":/icons/handle_up_arrow.svg"),
				   QPixmap(":/icons/handle_down_arrow.svg"), d_leftHandlesArea, true);
	chOffsetHdl->setRoundRectColor(chnColor);
	chOffsetHdl->setPen(QPen(chnColor, 2, Qt::SolidLine));
	chOffsetHdl->setVisible(true);
	pushBackNewOffsetWidgets(chOffsetHdl, chOffsetBar);

	connect(chOffsetHdl, &RoundedHandleV::positionChanged, [=](int pos) {
		int chn_id = m_offsetHandles.indexOf(chOffsetHdl);
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
	connect(chOffsetBar, &HorizBar::pixelPositionChanged, [=](int pos) { chOffsetHdl->setPositionSilenty(pos); });

	connect(chOffsetHdl, &RoundedHandleV::mouseReleased, [=]() {
		int chn_id = m_offsetHandles.indexOf(chOffsetHdl);
		int offset = this->VertOffset(chn_id);
		if (offset > m_maxOffsetValue) {
			offset = m_maxOffsetValue;
			this->setVertOffset(offset, chn_id);
			this->replot();

			Q_EMIT channelOffsetChanged(chn_id, offset);
		}
		if (offset < m_minOffsetValue) {
			offset = m_minOffsetValue;
			this->setVertOffset(offset, chn_id);
			this->replot();

			Q_EMIT channelOffsetChanged(chn_id, offset);
		}
	});

	connect(chOffsetHdl, &RoundedHandleV::reset, [=]() {
		int chn_id = m_offsetHandles.indexOf(chOffsetHdl);
		this->setVertOffset(0, chn_id);
		this->replot();

		Q_EMIT channelOffsetChanged(chn_id, 0);
	});

	/* Add Measure ojbect that handles all channel measurements */

	Measure* measure = nullptr;

	if (isReferenceWaveform(Curve(chnIdx))) {
		int idx = chnIdx - d_ydata.size();
		measure = new Measure(chnIdx, d_ref_ydata[idx], Curve(chnIdx)->data()->size(), nullptr);
	} else {
		int count = countReferenceWaveform(chnIdx);
		measure = new Measure(chnIdx, d_ydata[chnIdx - count], Curve(chnIdx)->data()->size(),
				      m_conversion_function);
	}

	measure->setAdcBitCount(12);
	m_measureObjs.push_back(measure);
}

void CapturePlot::computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate)
{
	if (chnIdx >= m_measureObjs.size()) {
		return;
	}

	Measure* measure = m_measureObjs[chnIdx];
	measure->setSampleRate(sampleRate);
	measure->measure();

	Q_EMIT measurementsAvailable();
}

void CapturePlot::setConversionFunction(const std::function<double(unsigned int, double, bool)>& fp)
{
	m_conversion_function = fp;
	for (int i = 0; i < m_measureObjs.size(); i++) {
		Measure* measure = m_measureObjs[i];
		measure->setConversionFunction(fp);
	}
}

void CapturePlot::cleanUpJustBeforeChannelRemoval(int chnIdx)
{
	Measure* measure = measureOfChannel(chnIdx);
	if (measure) {
		int pos = m_measureObjs.indexOf(measure);
		for (int i = pos + 1; i < m_measureObjs.size(); i++) {
			m_measureObjs[i]->setChannel(m_measureObjs[i]->channel() - 1);
		}
		m_measureObjs.removeOne(measure);
		delete measure;
	}
}

void CapturePlot::setOffsetWidgetVisible(int chnIdx, bool visible)
{
	if (chnIdx < 0 || chnIdx >= m_offsetHandles.size())
		return;

	m_offsetHandles[chnIdx]->setVisible(visible);

	// find the group of this handle
	auto hdlGroup =
		std::find_if(m_groupHandles.begin(), m_groupHandles.end(), [=](const QList<RoundedHandleV*>& group) {
			return group.contains(m_offsetHandles[chnIdx]);
		});

	// if no group return
	if (hdlGroup == m_groupHandles.end()) {
		qDebug() << "This handle is not in a group!";
		return;
	}

	int count = 0;

	auto hdlGroupContainer = *hdlGroup;
	for (const auto& handle : qAsConst(hdlGroupContainer)) {
		if (handle->isVisible()) {
			count++;
		}
	}

	const bool detach = (count < 2);
	const int groupIdx = m_groupHandles.indexOf(*hdlGroup);
	if (detach) {
		m_groupMarkers[groupIdx]->detach();
	} else {
		m_groupMarkers[groupIdx]->attach(this);
	}
}

void CapturePlot::removeOffsetWidgets(int chnIdx)
{
	if (chnIdx < 0 || chnIdx >= m_offsetHandles.size())
		return;

	HorizBar* bar = m_offsetBars.takeAt(chnIdx);
	bar->setMobileAxis(QwtAxisId(QwtPlot::yLeft, 0));
	d_symbolCtrl->detachSymbol(bar);
	delete bar;
	delete (m_offsetHandles.takeAt(chnIdx));
}

void CapturePlot::measure()
{
	for (int i = 0; i < m_measureObjs.size(); i++) {
		Measure* measure = m_measureObjs[i];
		if (measure->activeMeasurementsCount() > 0) {
			measure->setSampleRate(this->sampleRate());
			measure->measure();
		}
	}
}

int CapturePlot::activeMeasurementsCount(int chnIdx)
{
	int count = -1;
	Measure* measure = measureOfChannel(chnIdx);

	if (measure)
		count = measure->activeMeasurementsCount();

	return count;
}

void CapturePlot::onNewDataReceived()
{
	int ref_idx = 0;
	for (int i = 0; i < m_measureObjs.size(); i++) {
		Measure* measure = m_measureObjs[i];
		int chn = measure->channel();
		if (isReferenceWaveform(Curve(chn))) {
			measure->setDataSource(d_ref_ydata[ref_idx], Curve(chn)->data()->size());
			ref_idx++;
		} else {
			int count = countReferenceWaveform(chn);
			measure->setDataSource(d_ydata[chn - count], Curve(chn)->data()->size());
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
	Measure* measure = measureOfChannel(chnIdx);

	if (measure)
		return measure->measurments();
	else
		return QList<std::shared_ptr<MeasurementData>>();
}

std::shared_ptr<MeasurementData> CapturePlot::measurement(int id, int chnIdx)
{
	Measure* measure = measureOfChannel(chnIdx);
	if (measure)
		return measure->measurement(id);
	else
		return std::shared_ptr<MeasurementData>();
}

OscPlotZoomer* CapturePlot::getZoomer()
{
	if (d_zoomer.isEmpty())
		return nullptr;
	return static_cast<OscPlotZoomer*>(d_zoomer[0]);
}

void CapturePlot::setOffsetInterval(double minValue, double maxValue)
{
	m_minOffsetValue = minValue;
	m_maxOffsetValue = maxValue;
}

double CapturePlot::getMaxOffsetValue() { return m_maxOffsetValue; }

double CapturePlot::getMinOffsetValue() { return m_minOffsetValue; }

void CapturePlot::setPeriodDetectLevel(int chnIdx, double lvl)
{
	Measure* measure = measureOfChannel(chnIdx);
	if (measure)
		measure->setCrossLevel(lvl);
}

void CapturePlot::setPeriodDetectHyst(int chnIdx, double hyst)
{
	Measure* measure = measureOfChannel(chnIdx);
	if (measure)
		measure->setHysteresisSpan(hyst);
}

void CapturePlot::setTimeBaseLabelValue(double value)
{
	QString text = m_cursorTimeFormatter.format(value, "", 3);
	if (m_timeBaseLabel->text().contains(tr("Zoom: "))) {
		m_timeBaseLabel->setText(tr("Zoom: ") + text + tr("/div"));
	} else {
		m_timeBaseLabel->setText(text + tr("/div"));
	}
}

void CapturePlot::setTimeBaseZoomed(bool zoomed)
{
	if (zoomed) {
		if (!m_timeBaseLabel->text().contains(tr("Zoom: ")))
			m_timeBaseLabel->setText(tr("Zoom: ") + m_timeBaseLabel->text());
	} else {
		QString text = m_timeBaseLabel->text();
		if (text.contains(tr("Zoom: "))) {
			text = text.remove(tr("Zoom: "));
			m_timeBaseLabel->setText(text);
		}
	}
}

void CapturePlot::setBufferSizeLabelValue(int numSamples)
{
	m_bufferSizeLabelVal = numSamples;
	updateBufferSizeSampleRateLabel(numSamples, m_sampleRateLabelVal);
}

void CapturePlot::setSampleRatelabelValue(double sampleRate)
{
	m_sampleRateLabelVal = sampleRate;
	updateBufferSizeSampleRateLabel(m_bufferSizeLabelVal, sampleRate);
}

void CapturePlot::setTriggerState(int triggerState)
{
	m_triggerStateLabel->hide();

	switch (triggerState) {
	case Waiting:
		m_triggerStateLabel->setText(tr("Waiting"));
		break;
	case Triggered:
		m_triggerStateLabel->setText(tr("Triggered"));
		break;
	case Stop:
		m_triggerStateLabel->setText(tr("Stop"));
		break;
	case Auto:
		m_triggerStateLabel->setText(tr("Auto"));
		break;
	default:
		break;
	};
	m_triggerStateLabel->show();
}

void CapturePlot::setMaxBufferSizeErrorLabel(bool reached, const QString& customWarning)
{
	QString errorMessage = "Maximum buffer size reached";
	if (customWarning.length()) {
		errorMessage = customWarning;
	}
	m_maxBufferError->setText(reached ? errorMessage : "");
}

void CapturePlot::updateBufferSizeSampleRateLabel(int nsamples, double sr)
{
	QString txtSampleRate = m_cursorMetricFormatter.format(sr, "sps", 0);
	m_cursorMetricFormatter.setTrimZeroes(true);
	QString txtSamples = m_cursorMetricFormatter.format(nsamples, "", 3);
	m_cursorMetricFormatter.setTrimZeroes(false);
	QString text = QString("%1 Samples at ").arg(txtSamples) + txtSampleRate;
	m_sampleRateLabel->setText(text);
}

void CapturePlot::removeLeftVertAxis(unsigned int axis)
{
	const unsigned int numAxis = vertAxes.size();

	if (axis >= numAxis)
		return;

	// Update the mobile axis ID of all symbols
	for (int i = axis; i < numAxis - 1; i++) {
		QwtAxisId axisId = m_offsetBars.at(i)->mobileAxis();
		--axisId.id;
		m_offsetBars.at(i)->setMobileAxis(axisId);
	}

	DisplayPlot::removeLeftVertAxis(axis);
}
