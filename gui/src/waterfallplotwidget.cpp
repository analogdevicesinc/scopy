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

#include "waterfallplotwidget.h"

#include "plotaxis.h"
#include "style.h"

#include <QMouseEvent>
#include <QDebug>
#include <QwtPlotCanvas>
#include <QwtPlotLayout>
#include <QwtPlotOpenGLCanvas>
#include <qwt_scale_widget.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>

#include <hoverwidget.h>
#include <plotbuttonmanager.h>
#include <plotinfo.h>
#include <osc_scale_engine.h>
#include <pluginbase/preferences.h>
#include <stylehelper.h>

#include <cfloat>
#include <algorithm>

using namespace scopy;

// =============================================================================
// WaterfallData
// =============================================================================

WaterfallData::WaterfallData(int maxRows)
	: QwtRasterData()
	, m_maxRows(maxRows)
	, m_fftSize(0)
	, m_xInterval(0.0, 1.0)
	, m_yInterval(0.0, static_cast<double>(maxRows))
	, m_zInterval(-120.0, 0.0)
{
}

WaterfallData::~WaterfallData() {}

void WaterfallData::addFFTData(const double *data, size_t size)
{
	if(!data || size == 0)
		return;

	// First call sets the FFT size
	if(m_fftSize == 0)
		m_fftSize = size;

	// If FFT size changed, clear existing data to stay consistent
	if(size != m_fftSize) {
		m_data.clear();
		m_fftSize = size;
	}

	m_data.push_back(std::vector<double>(data, data + size));

	while(static_cast<int>(m_data.size()) > m_maxRows)
		m_data.pop_front();
}

void WaterfallData::reset()
{
	m_data.clear();
}

void WaterfallData::setXInterval(double minFreq, double maxFreq)
{
	m_xInterval = QwtInterval(minFreq, maxFreq);
}

void WaterfallData::setYInterval(double min, double max)
{
	m_yInterval = QwtInterval(min, max);
}

void WaterfallData::setZInterval(double minDb, double maxDb)
{
	m_zInterval = QwtInterval(minDb, maxDb);
}

void WaterfallData::setMaxRows(int rows)
{
	if(rows <= 0)
		return;
	m_maxRows = rows;
	m_data.clear();
}

int WaterfallData::maxRows() const { return m_maxRows; }

int WaterfallData::rowCount() const { return static_cast<int>(m_data.size()); }

QwtInterval WaterfallData::interval(Qt::Axis axis) const
{
	switch(axis) {
	case Qt::XAxis: return m_xInterval;
	case Qt::YAxis: return m_yInterval;
	case Qt::ZAxis: return m_zInterval;
	default:        return QwtInterval();
	}
}

double WaterfallData::value(double x, double y) const
{
	if(m_data.empty() || m_fftSize == 0)
		return -DBL_MAX;

	const int nRows = static_cast<int>(m_data.size());

	// Map Y coordinate to row index.
	// Y=yMin → row 0 (oldest), Y=yMax → row nRows-1 (newest).
	const double yMin = m_yInterval.minValue();
	const double yMax = m_yInterval.maxValue();
	const double yRange = yMax - yMin;
	if(yRange <= 0.0)
		return -DBL_MAX;

	// row index: 0 = bottom-most displayed row
	const double rowFrac = (y - yMin) / yRange * static_cast<double>(m_maxRows);
	const int row = static_cast<int>(rowFrac);

	// Rows in m_data: index 0 = oldest, index nRows-1 = newest.
	// In the display the newest row is at the top (highest Y).
	// The "blank" area at the bottom has no data yet.
	const int blankRows = m_maxRows - nRows;
	const int dataRow = row - blankRows;

	if(dataRow < 0 || dataRow >= nRows)
		return -DBL_MAX;

	// Map X coordinate to FFT bin index
	const double xMin = m_xInterval.minValue();
	const double xMax = m_xInterval.maxValue();
	const double xRange = xMax - xMin;
	if(xRange <= 0.0)
		return -DBL_MAX;

	const double binFrac = (x - xMin) / xRange * static_cast<double>(m_fftSize - 1);
	const int bin = static_cast<int>(binFrac + 0.5);

	if(bin < 0 || static_cast<size_t>(bin) >= m_fftSize)
		return -DBL_MAX;

	return m_data[static_cast<size_t>(dataRow)][static_cast<size_t>(bin)];
}

// =============================================================================
// WaterfallPlotWidget
// =============================================================================

WaterfallPlotWidget::WaterfallPlotWidget(QWidget *parent)
	: QWidget(parent)
	, m_startFreq(0.0)
	, m_stopFreq(1.0)
	, m_showXAxisLabels(true)
	, m_showYAxisLabels(true)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_plot = new QwtPlot(this);
	m_plot->canvas()->setContentsMargins(0, 0, 0, 0);

	m_layout = new QGridLayout(this);
	m_layout->addWidget(m_plot, 1, 0);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);

	m_plot->plotLayout()->setAlignCanvasToScales(true);
	m_plot->plotLayout()->setCanvasMargin(0);
	m_plot->plotLayout()->setSpacing(0);

	// --- Spectrogram setup ---
	m_data = new WaterfallData(200);
	m_data->setXInterval(m_startFreq, m_stopFreq);
	m_data->setYInterval(0.0, 200.0);
	m_data->setZInterval(-120.0, 0.0);

	m_spectrogram = new QwtPlotSpectrogram();
	m_spectrogram->setData(m_data);
	m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
	m_spectrogram->setColorMap(new WaterfallColorMap());
	m_spectrogram->attach(m_plot);

	// --- Infrastructure setup ---
	setupOpenGLCanvas();
	setupAxes();
	setupColorBar();
	setupNavigator();
	setupPlotInfo();
	setupPlotButtonManager();

	// Style canvas background
	Style::setBackgroundColor(m_plot->canvas(), json::theme::background_plot, true);

	m_plot->canvas()->installEventFilter(this);
}

WaterfallPlotWidget::~WaterfallPlotWidget() {}

// -------------------------------------------------------------------------
// Setup helpers
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setupOpenGLCanvas()
{
	bool useOpenGL = Preferences::GetInstance()->get("general_use_opengl").toBool();
	if(useOpenGL) {
		QwtPlotOpenGLCanvas *c = qobject_cast<QwtPlotOpenGLCanvas *>(m_plot->canvas());
		if(!c) {
			c = new QwtPlotOpenGLCanvas(m_plot);
			c->setPaintAttribute(QwtPlotAbstractGLCanvas::BackingStore);
			m_plot->setCanvas(c);
		}
	} else {
		QwtPlotCanvas *c = qobject_cast<QwtPlotCanvas *>(m_plot->canvas());
		if(c)
			c->setPaintAttribute(QwtPlotCanvas::BackingStore, true);
	}
}

void WaterfallPlotWidget::setupAxes()
{
	// Hide the default QwtPlot axes
	m_plot->setAxisVisible(QwtAxis::YLeft, false);
	m_plot->setAxisVisible(QwtAxis::XBottom, false);
	m_plot->setAxisVisible(QwtAxis::YRight, false);

	m_xPosition = Preferences::get("adc_plot_xaxis_label_position").toInt();
	m_yPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();

	QPen pen(QColor(Style::getAttribute(json::theme::content_subtle)));

	// X axis — frequency
	m_xAxis = new PlotAxis(m_xPosition, m_plot, 0, pen, this);
	m_xAxis->setInterval(m_startFreq, m_stopFreq);
	addPlotAxis(m_xAxis);

	// Y axis — time / row index
	m_yAxis = new PlotAxis(m_yPosition, m_plot, 0, pen, this);
	m_yAxis->setInterval(0.0, 200.0);
	addPlotAxis(m_yAxis);
}

void WaterfallPlotWidget::setupNavigator()
{
	// Start with an empty axis set; addAxis will populate it and create navigators.
	QSet<QwtAxisId> *axes = new QSet<QwtAxisId>();
	m_navigator = new PlotNavigator(m_plot, axes, this);

	// addAxis registers each axis, creates magnifier/zoomer, and connects
	// the axis scale-updated signal for auto base-rect tracking.
	m_navigator->addAxis(m_xAxis);
	m_navigator->addAxis(m_yAxis);

	connect(m_navigator, &PlotNavigator::rectChanged, this, &WaterfallPlotWidget::plotScaleChanged);
}

void WaterfallPlotWidget::setupPlotInfo()
{
	m_plotInfo = new PlotInfo(m_plot->canvas());
}

void WaterfallPlotWidget::setupPlotButtonManager()
{
	m_plotButtonManager = new PlotButtonManager(this);
	m_plotButtonManager->setCollapseOrientation(PlotButtonManager::PBM_RIGHT);

	HoverWidget *hover = new HoverWidget(m_plotButtonManager, m_plot->canvas(), m_plot->canvas());
	hover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	hover->setContentPos(HoverPosition::HP_TOPLEFT);
	hover->setAnchorOffset(QPoint(0, -20));
	hover->setRelative(true);
	hover->show();
}

void WaterfallPlotWidget::setupColorBar()
{
	// Enable the right axis as a color bar
	m_plot->setAxisVisible(QwtAxis::YRight, true);
	updateColorBar();
}

void WaterfallPlotWidget::updateColorBar()
{
	QwtScaleWidget *rightAxis = m_plot->axisWidget(QwtAxis::YRight);
	if(!rightAxis)
		return;

	QwtInterval intv = m_data->interval(Qt::ZAxis);
	rightAxis->setColorBarEnabled(true);
	rightAxis->setColorMap(intv, new WaterfallColorMap());

	m_plot->setAxisScale(QwtAxis::YRight, intv.minValue(), intv.maxValue());
	m_plot->plotLayout()->setAlignCanvasToScales(true);

	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

// -------------------------------------------------------------------------
// Core accessors
// -------------------------------------------------------------------------

QwtPlot *WaterfallPlotWidget::plot() const { return m_plot; }

QGridLayout *WaterfallPlotWidget::layout() { return m_layout; }

PlotAxis *WaterfallPlotWidget::xAxis() { return m_xAxis; }

PlotAxis *WaterfallPlotWidget::yAxis() { return m_yAxis; }

void WaterfallPlotWidget::addPlotAxis(PlotAxis *ax)
{
	m_plotAxis[ax->position()].append(ax);
}

QList<PlotAxis *> &WaterfallPlotWidget::plotAxis(int position)
{
	return m_plotAxis[position];
}

PlotAxis *WaterfallPlotWidget::plotAxisFromId(QwtAxisId axisId)
{
	for(QList<PlotAxis *> &axes : m_plotAxis) {
		for(PlotAxis *axis : axes) {
			if(axis->axisId() == axisId)
				return axis;
		}
	}
	return nullptr;
}

PlotNavigator *WaterfallPlotWidget::navigator() const { return m_navigator; }

PlotButtonManager *WaterfallPlotWidget::plotButtonManager() const { return m_plotButtonManager; }

PlotInfo *WaterfallPlotWidget::getPlotInfo() { return m_plotInfo; }

// -------------------------------------------------------------------------
// Data API
// -------------------------------------------------------------------------

void WaterfallPlotWidget::addFFTData(const double *data, size_t size)
{
	m_data->addFFTData(data, size);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
	Q_EMIT newData();
}

void WaterfallPlotWidget::clearData()
{
	m_data->reset();
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
	replot();
}

// -------------------------------------------------------------------------
// Frequency axis
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setFrequencyRange(double startHz, double stopHz)
{
	m_startFreq = startHz;
	m_stopFreq = stopHz;
	m_data->setXInterval(startHz, stopHz);
	m_xAxis->setInterval(startHz, stopHz);

	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

double WaterfallPlotWidget::startFrequency() const { return m_startFreq; }

double WaterfallPlotWidget::stopFrequency() const { return m_stopFreq; }

// -------------------------------------------------------------------------
// Y axis
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setYRange(double min, double max)
{
	m_data->setYInterval(min, max);
	m_yAxis->setInterval(min, max);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

// -------------------------------------------------------------------------
// Intensity / color map
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setIntensityRange(double minDb, double maxDb)
{
	m_data->setZInterval(minDb, maxDb);
	updateColorBar();
}

double WaterfallPlotWidget::minIntensity() const
{
	return m_data->interval(Qt::ZAxis).minValue();
}

double WaterfallPlotWidget::maxIntensity() const
{
	return m_data->interval(Qt::ZAxis).maxValue();
}

void WaterfallPlotWidget::autoScaleIntensity()
{
	// Placeholder — a full implementation would iterate m_data rows
	// and find the min/max values, then call setIntensityRange.
	// Left for the caller to implement via the WaterfallData API if needed.
}

// -------------------------------------------------------------------------
// Row count
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setNumRows(int rows)
{
	if(rows <= 0)
		return;
	m_data->setMaxRows(rows);
	// Update Y axis and data range to match
	const double yMax = m_yAxis->max();
	const double yMin = m_yAxis->min();
	m_data->setYInterval(yMin, yMax);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

int WaterfallPlotWidget::numRows() const { return m_data->maxRows(); }

// -------------------------------------------------------------------------
// Color map
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setColorMap(QwtColorMap *colorMap)
{
	m_spectrogram->setColorMap(colorMap);
	updateColorBar();
	replot();
}

// -------------------------------------------------------------------------
// Color bar
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setColorBarVisible(bool visible)
{
	m_plot->setAxisVisible(QwtAxis::YRight, visible);
}

void WaterfallPlotWidget::setColorBarTitle(const QString &title)
{
	QwtScaleWidget *rightAxis = m_plot->axisWidget(QwtAxis::YRight);
	if(rightAxis)
		rightAxis->setTitle(QwtText(title));
}

// -------------------------------------------------------------------------
// Axis label visibility
// -------------------------------------------------------------------------

bool WaterfallPlotWidget::showXAxisLabels() const { return m_showXAxisLabels; }

void WaterfallPlotWidget::setShowXAxisLabels(bool visible)
{
	m_showXAxisLabels = visible;
	showAxisLabels();
}

bool WaterfallPlotWidget::showYAxisLabels() const { return m_showYAxisLabels; }

void WaterfallPlotWidget::setShowYAxisLabels(bool visible)
{
	m_showYAxisLabels = visible;
	showAxisLabels();
}

void WaterfallPlotWidget::showAxisLabels()
{
	m_xAxis->setVisible(m_showXAxisLabels);
	m_yAxis->setVisible(m_showYAxisLabels);
}

void WaterfallPlotWidget::hideAxisLabels()
{
	m_xAxis->setVisible(false);
	m_yAxis->setVisible(false);
}

void WaterfallPlotWidget::setUnitsVisible(bool visible)
{
	m_xAxis->setUnitsVisible(visible);
	m_yAxis->setUnitsVisible(visible);
}

// -------------------------------------------------------------------------
// Misc
// -------------------------------------------------------------------------

void WaterfallPlotWidget::setAlignCanvasToScales(bool align)
{
	m_plot->plotLayout()->setAlignCanvasToScales(align);
}

void WaterfallPlotWidget::replot()
{
	m_plot->replot();
}

bool WaterfallPlotWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == m_plot->canvas()) {
		switch(event->type()) {
		case QEvent::MouseMove:
			Q_EMIT mouseMove(static_cast<QMouseEvent *>(event));
			break;
		case QEvent::MouseButtonPress:
			Q_EMIT mouseButtonPress(static_cast<QMouseEvent *>(event));
			break;
		case QEvent::MouseButtonRelease:
			Q_EMIT mouseButtonRelease(static_cast<QMouseEvent *>(event));
			break;
		case QEvent::Resize:
			Q_EMIT canvasSizeChanged();
			break;
		default:
			break;
		}
	}
	return QObject::eventFilter(object, event);
}

QSize WaterfallPlotWidget::minimumSizeHint() const { return QSize(0, 0); }

#include "moc_waterfallplotwidget.cpp"
