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

#include "FftDisplayPlot.h"
#include "spectrumUpdateEvents.h"
#include "signal_generator.hpp"
#include "average.h"
#include "spectrum_marker.hpp"
#include "marker_controller.h"
#include "limitedplotzoomer.h"
#include "osc_scale_engine.h"

#include <QDebug>
#include <qwt_symbol.h>
#include <boost/make_shared.hpp>

#define ERROR_VALUE -10000000

using namespace adiscope;

class FftDisplayZoomer: public LimitedPlotZoomer
{
public:
#if QWT_VERSION < 0x060100
  FftDisplayZoomer(QwtPlotCanvas* canvas)
#else /* QWT_VERSION < 0x060100 */
  FftDisplayZoomer(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
    : LimitedPlotZoomer(canvas)
  {
    setTrackerMode(QwtPicker::AlwaysOn);
  }

  virtual ~FftDisplayZoomer(){

  }

  virtual void updateTrackerText(){
    updateDisplay();
  }

protected:
  using QwtPlotZoomer::trackerText;
  virtual QwtText trackerText( const QPoint& p ) const
  {
    QPointF dp = QwtPlotZoomer::invTransform(p);
    QwtText t(QString("(%1, %2)").arg(dp.x(), 0, 'f', 4).
              arg(dp.y(), 0, 'f', 4));
    return t;
  }
};

FftDisplayPlot::FftDisplayPlot(int nplots, QWidget *parent) :
	DisplayPlot(nplots, parent),
	d_start_frequency(0),
	d_stop_frequency(1000),
	d_sampl_rate(1),
	d_preset_sampl_rate(d_sampl_rate),
	d_presetMagType(MagnitudeType::DBFS),
	d_mrkCtrl(nullptr),
	d_emitNewMkrData(true),
	m_visiblePeakSearch(true),
	d_logScaleEnabled(false),
	d_buffer_idx(0),
	d_nb_overlapping_avg(1),
	n_ref_curves(0)
{
	// TO DO: Add more colors
	d_markerColors << QColor(255, 242, 0) << QColor(210, 155, 210);
	d_zoomer.push_back(nullptr);

	for (unsigned int i = 0; i < nplots; i++) {
		auto plot = new QwtPlotCurve(QString("CH %1").arg(i + 1));
		plot->setPaintAttribute(QwtPlotCurve::ClipPolygons);
		plot->setPaintAttribute(QwtPlotCurve::ImageBuffer);
		plot->setPaintAttribute(QwtPlotCurve::FilterPoints);
		plot->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive);

		plot->setPen(QPen(d_CurveColors[i]));
		plot->attach(this);

		d_plot_curve.push_back(plot);
		y_data.push_back(nullptr);
		y_original_data.push_back(nullptr);

		d_ch_average_type.push_back(AverageType::SAMPLE);

		d_num_markers.push_back(0);
		d_markers.push_back(QList<marker>());
		d_peaks.push_back(
			QList<std::shared_ptr<marker_data>>());
		d_freq_asc_sorted_peaks.push_back(
			QList<std::shared_ptr<marker_data>>());
		d_current_avg_index.push_back(0);
	}
	y_scale_factor.resize(nplots);
	d_ch_avg_obj.resize(nplots);
	d_win_coefficient_sum_sqr.resize(nplots);
	d_win_coefficient_sum.resize(nplots);

	m_sweepStart = 0;
	m_sweepStop = 1000;
	d_magType = MagnitudeType::DBFS;

	d_numPoints = 1024;
	x_data = new double[d_numPoints];

	d_firstInit = true;

	dBFormatter.setTwoDecimalMode(false);
	freqFormatter.setTwoDecimalMode(true);

	OscScaleDraw *yScaleDraw = new OscScaleDraw(&dBFormatter, "");
	setAxisScaleDraw(QwtAxis::YLeft, yScaleDraw);
	yScaleDraw->setFloatPrecision(2);

	OscScaleDraw *xScaleDraw = new OscScaleDraw(&freqFormatter, "");
	setAxisScaleDraw(QwtAxis::XBottom, xScaleDraw);
	xScaleDraw->setFloatPrecision(2);

	_resetXAxisPoints();

	d_mrkCtrl = new MarkerController(this);

	connect(d_mrkCtrl,
		SIGNAL(markerSelected(std::shared_ptr<SpectrumMarker>&)),
		this,
		SLOT(onMrkCtrlMarkerSelected(std::shared_ptr<SpectrumMarker>&))
	);
	connect(d_mrkCtrl,
		SIGNAL(markerPositionChanged(std::shared_ptr<SpectrumMarker>&)),
		this,
		SLOT(onMrkCtrlMarkerPosChanged(std::shared_ptr<SpectrumMarker>&))
	);
	connect(d_mrkCtrl,
		SIGNAL(markerReleased(std::shared_ptr<SpectrumMarker>&)),
		this,
		SLOT(onMrkCtrlMarkerReleased(std::shared_ptr<SpectrumMarker>&))
	);


	setMinXaxisDivision(1);     // A minimum division of 1 Hz
	setMaxXaxisDivision(5E6);   // A maximum division of 5 MHz
	setMinYaxisDivision(1E-3);  // A minimum division of 1 mdB
	setMaxYaxisDivision(100); // A maximum division of 100 dB
	setVertUnitsPerDiv(20);
	setVertOffset(-VertUnitsPerDiv() * 5);

	setYaxisNumDiv(11);

	d_selected_channel = 0;

	d_topHandlesArea->setMinimumHeight(50);
	d_topHandlesArea->setLargestChildWidth(50);

	d_leftHandlesArea->setMinimumWidth(50);
	d_leftHandlesArea->setTopPadding(50);
	d_leftHandlesArea->setBottomPadding(55);
	d_leftHandlesArea->setMinimumHeight(this->minimumHeight());

	setAxisVisible(QwtAxis::XBottom, false);
	setAxisVisible(QwtAxis::YLeft, false);
	d_formatter = static_cast<PrefixFormatter *>(new MetricPrefixFormatter);

	setupReadouts();

	installEventFilter(this);
}

FftDisplayPlot::~FftDisplayPlot()
{
	for (uint c = 0; c < d_nplots + n_ref_curves; c++) {
		for (uint i = 0; i < d_markers[c].size(); i++) {
			d_markers[c][i].ui->detach();
		}
	}

	if (x_data)
		delete[] x_data;

	for (unsigned int i = 0; i < d_nplots; i++) {
		if (y_data[i])
			delete[] y_data[i];
		if (y_original_data[i])
			delete[] y_original_data[i];
	}

	for (unsigned int i = 0; i < n_ref_curves; ++i) {
		delete d_refXdata[i];
		delete d_refYdata[i];
	}
	d_refXdata.clear();
	d_refYdata.clear();
	removeEventFilter(this);
	canvas()->removeEventFilter(d_cursorReadouts);
	canvas()->removeEventFilter(d_symbolCtrl);
}

void FftDisplayPlot::initChannelMeasurement(int nplots) {
     Q_EMIT channelAdded(nplots);
}

void FftDisplayPlot::replot()
{
	if (!d_leftHandlesArea || !d_bottomHandlesArea) {
		return;
	}

	d_leftHandlesArea->update();
	d_bottomHandlesArea->update();

	BasicPlot::replot();
}

bool FftDisplayPlot::isReferenceWaveform(unsigned int chnIdx)
{
    QwtPlotCurve *curve = Curve(chnIdx);
    return d_ref_curves.values().contains(curve);
}

size_t FftDisplayPlot::getCurveSize(unsigned int chnIdx)
{
    return Curve(chnIdx)->data()->size();
}

QString FftDisplayPlot::formatXValue(double value, int precision) const
{
	return d_formatter->format(value, d_xAxisUnit, precision);
}

QString FftDisplayPlot::formatYValue(double value, int precision) const
{
	return d_formatter->format(value, d_yAxisUnit, precision);
}

void FftDisplayPlot::setupReadouts()
{
	d_cursorReadouts->setTimeReadoutVisible(false);
	d_cursorReadouts->setVoltageReadoutVisible(false);

	d_cursorReadouts->setTimeCursor1LabelText("F1 = ");
	d_cursorReadouts->setTimeCursor2LabelText("F2 = ");
	d_cursorReadouts->setTimeDeltaLabelText("ΔF = ");
	d_cursorReadouts->setVoltageCursor1LabelText("Mag1 = ");
	d_cursorReadouts->setVoltageCursor2LabelText("Mag2 = ");
	d_cursorReadouts->setDeltaVoltageLabelText("ΔMag = ");

	d_cursorReadouts->setFrequencyDeltaVisible(false);
	d_cursorReadouts->setTransparency(0);
}

void FftDisplayPlot::updateHandleAreaPadding()
{
	d_leftHandlesArea->update();
	d_bottomHandlesArea->setLeftPadding(d_leftHandlesArea->width());
	d_bottomHandlesArea->setRightPadding(50);

	d_rightHandlesArea->setTopPadding(50);
	d_rightHandlesArea->setBottomPadding(50);

	//update handle position to avoid cursors getting out of the plot bounds when changing the padding;
	d_hCursorHandle1->updatePosition();
	d_hCursorHandle2->updatePosition();
	d_vCursorHandle1->updatePosition();
	d_vCursorHandle2->updatePosition();
}

void FftDisplayPlot::onHCursor1Moved(double value)
{
	QString text;
	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = d_formatter->format(value, "dB", 3);
	d_cursorReadouts->setVoltageCursor1Text(error ? "-" : text);
	d_cursorReadoutsText.t1 = error ? "-" : text;

	double valueCursor2;
	if (d_trackMode) {
		valueCursor2 = getHorizontalCursorIntersection(d_vBar2->plotCoord().x());
	} else {
		valueCursor2 = d_hBar2->plotCoord().y();
	}

	double diff = value - (valueCursor2 * d_displayScale);
	text = d_formatter->format(diff, "dB", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.tDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void FftDisplayPlot::onHCursor2Moved(double value)
{
	QString text;
	bool error = false;
	if (d_trackMode) {
		if (value == ERROR_VALUE) {
			error = true;
		}
	}

	value *= d_displayScale;
	text = d_formatter->format(value, "dB", 3);
	d_cursorReadouts->setVoltageCursor2Text(error ? "-" : text);
	d_cursorReadoutsText.t2 = error ? "-" : text;

	double valueCursor1;
	if (d_trackMode) {
		valueCursor1 = getHorizontalCursorIntersection(d_vBar1->plotCoord().x());
	} else {
		valueCursor1 = d_hBar1->plotCoord().y();
	}

	double diff = (valueCursor1 * d_displayScale) - value;
	text = d_formatter->format(diff, "dB", 3);
	d_cursorReadouts->setVoltageDeltaText(error ? "-" : text);
	d_cursorReadoutsText.tDelta = error ? "-" : text;

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void FftDisplayPlot::onVCursor1Moved(double value)
{
	QString text;
	text = d_formatter->format(value, "Hz", 3);
	d_cursorReadouts->setTimeCursor1Text(text);
	d_cursorReadoutsText.v1 = text;

	double diff = value - d_vBar2->plotCoord().x();
	text = d_formatter->format(diff, "Hz", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	if (d_trackMode) {
		onHCursor1Moved(getHorizontalCursorIntersection(d_vBar1->plotCoord().x()));
	}

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void FftDisplayPlot::onVCursor2Moved(double value)
{
	QString text;
	text = d_formatter->format(value, "Hz", 3);
	d_cursorReadouts->setTimeCursor2Text(text);
	d_cursorReadoutsText.v2 = text;

	double diff = d_vBar1->plotCoord().x() - value;
	text = d_formatter->format(diff, "Hz", 3);
	d_cursorReadouts->setTimeDeltaText(text);
	d_cursorReadoutsText.vDelta = text;

	if (d_trackMode) {
		onHCursor2Moved(getHorizontalCursorIntersection(d_vBar2->plotCoord().x()));
	}

	Q_EMIT cursorReadoutsChanged(d_cursorReadoutsText);
}

void FftDisplayPlot::enableXaxisLabels()
{
	d_bottomHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new XBottomRuller(this)));
}

void FftDisplayPlot::enableYaxisLabels()
{
	d_leftHandlesArea->installExtension(std::unique_ptr<HandlesAreaExtension>(new YLeftRuller(this)));
}

bool FftDisplayPlot::eventFilter(QObject *object, QEvent *event)
{
	if (object == canvas() && event->type() == QEvent::Resize) {
		updateHandleAreaPadding();

		//force cursor handles to emit position changed
		//when the plot canvas is being resized
		d_hCursorHandle1->triggerMove();
		d_hCursorHandle2->triggerMove();
		d_vCursorHandle1->triggerMove();
		d_vCursorHandle2->triggerMove();

	}
	return QObject::eventFilter(object, event);
}

void FftDisplayPlot::showEvent(QShowEvent *event)
{
	d_vCursorHandle1->triggerMove();
	d_vCursorHandle2->triggerMove();
	d_hCursorHandle1->triggerMove();
	d_hCursorHandle2->triggerMove();
}

void FftDisplayPlot::setSelectedChannel(int id)
{
	if (d_selected_channel != id)  {
		d_selected_channel = id;
	}
}

void FftDisplayPlot::setZoomerEnabled()
{
	setAxisVisible(QwtAxis::XBottom, true);
	setAxisVisible(QwtAxis::YLeft, true);
	if(!d_zoomer[0]) {
		d_zoomer[0] = new FftDisplayZoomer(canvas());

                QFont font;
                font.setPointSize(10);
                font.setWeight(75);
		d_zoomer[0]->setTrackerFont(font);

#if QWT_VERSION < 0x060000
		d_zoomer[0]->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

		d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect2,
                                          Qt::RightButton, Qt::ControlModifier);
		d_zoomer[0]->setMousePattern(QwtEventPattern::MouseSelect3,
                                          Qt::RightButton);

                const QColor c("#999999");
		d_zoomer[0]->setRubberBandPen(c);
		d_zoomer[0]->setTrackerPen(c);
        }
}

void FftDisplayPlot::setNumPoints(uint64_t num_points)
{
	d_numPoints = num_points;
}

QColor FftDisplayPlot::getChannelColor()
{
	for (QList<QColor>::const_iterator it = d_CurveColors.cbegin();
			it != d_CurveColors.cend(); ++it) {
		bool used = false;

		for (std::vector<QwtPlotCurve *>::const_iterator it2 = d_plot_curve.cbegin();
				!used && it2 != d_plot_curve.cend(); ++it2)
			used = (*it2)->pen().color() == (*it);

		if (!used)
			return *it;
	}

	return Qt::black;
}

void FftDisplayPlot::registerReferenceWaveform(QString name, QVector<double> xData, QVector<double> yData)
{
	QColor color = getChannelColor();

	QwtPlotCurve *curve = new QwtPlotCurve();
	curve->setSamples(xData, yData);
	d_refXdata.push_back(new double[xData.size()]);
	memcpy(d_refXdata.back(), xData.data(), sizeof(double) * xData.size());
	d_refYdata.push_back(new double[yData.size()]);
	memcpy(d_refYdata.back(), yData.data(), sizeof(double) * yData.size());

	curve->setPen(QPen(color));
	curve->setRenderHint(QwtPlotItem::RenderAntialiased);

	QwtSymbol *symbol = new QwtSymbol(QwtSymbol::NoSymbol, QBrush(color),
					QPen(color), QSize(7,7));

	curve->setSymbol(symbol);

	curve->attach(this);

	d_ref_curves.insert(name, curve);
	d_plot_curve.push_back(curve);

	d_num_markers.push_back(0);
	d_markers.push_back(QList<marker>());
	d_peaks.push_back(
		QList<std::shared_ptr<marker_data>>());
	d_freq_asc_sorted_peaks.push_back(
		QList<std::shared_ptr<marker_data>>());

	setMarkerCount(d_num_markers.size() - 1, 5);
	for (int m = 0; m < 5; m++) {
		setMarkerEnabled(d_num_markers.size() - 1, m, false);
	}
	setPeakCount(d_num_markers.size() - 1, 10);

	findPeaks(d_plot_curve.size() - 1);

	Q_EMIT channelAdded(y_data.size() + n_ref_curves);
	n_ref_curves++;

	replot();
}

void FftDisplayPlot::unregisterReferenceWaveform(QString name)
{
	int chIdx = -1;

	for (size_t i = 0; i < d_plot_curve.size(); ++i) {
		if (d_plot_curve[i] == d_ref_curves[name]) {
			chIdx = i;
		}
	}

	auto it = std::find(d_plot_curve.begin(), d_plot_curve.end(), d_ref_curves[name]);

	if (it == d_plot_curve.end()) {
		return;
	}

	d_plot_curve.erase(std::find(d_plot_curve.begin(), d_plot_curve.end(), d_ref_curves[name]));

	QwtPlotCurve *curve = d_ref_curves[name];
	d_ref_curves.remove(name);

	curve->detach();
	delete curve;

	setMarkerCount(chIdx, 0);
	d_markers.removeAt(chIdx);
	d_num_markers.removeAt(chIdx);
	d_peaks.removeAt(chIdx);
	d_freq_asc_sorted_peaks.removeAt(chIdx);

	n_ref_curves--;

	replot();
}

void FftDisplayPlot::setWindowCoefficientSum(unsigned int ch, float sum, float sqr_sum)
{
	d_win_coefficient_sum[ch] = sum;
	d_win_coefficient_sum_sqr[ch] = sqr_sum;
}

void FftDisplayPlot::useLogScaleY(bool log_scale)
{
	if (log_scale) {
		setPlotYLogaritmic(true);
		QwtLogScaleEngine *scaleEngine = new QwtLogScaleEngine();
		setAxisScaleEngine(QwtAxis::YLeft,  (QwtScaleEngine *)scaleEngine);
//		OscScaleDraw *yScaleDraw = new OscScaleDraw(&dBFormatter, "V/√Hz");
//		yScaleDraw->enableComponent(QwtAbstractScaleDraw::Ticks, true);
//		yScaleDraw->setFloatPrecision(2);
//		setAxisScaleDraw(QwtAxis::YLeft, yScaleDraw);
		replot();
		auto div = axisScaleDiv(QwtAxis::YLeft);
		setYaxisMajorTicksPos(div.ticks(2));
	} else {
		setPlotYLogaritmic(false);
		OscScaleEngine *scaleEngine = new OscScaleEngine();
		this->setAxisScaleEngine(QwtAxis::YLeft, (QwtScaleEngine *)scaleEngine);
//		OscScaleDraw *yScaleDraw = new OscScaleDraw(&dBFormatter, "");
//		yScaleDraw->setFloatPrecision(2);
//		setAxisScaleDraw(QwtAxis::YLeft, yScaleDraw);
		replot();
		auto div = axisScaleDiv(QwtAxis::YLeft);
		setYaxisNumDiv((div.ticks(2)).size());
	}

	replot();
}

void FftDisplayPlot::useLogFreq(bool use_log_freq)
{
	if (use_log_freq) {
		setPlotLogaritmic(true);
		setAxisScaleEngine(QwtAxis::XBottom, new QwtLogScaleEngine);
		replot();
		auto div = axisScaleDiv(QwtAxis::XBottom);
		setXaxisMajorTicksPos(div.ticks(2));
	} else {
		setPlotLogaritmic(false);
		OscScaleEngine *scaleEngine = new OscScaleEngine();
		this->setAxisScaleEngine(QwtAxis::XBottom, (QwtScaleEngine *)scaleEngine);
		replot();
		auto div = axisScaleDiv(QwtAxis::XBottom);
		setXaxisNumDiv((div.ticks(2)).size() - 1);
	}
	d_logScaleEnabled = use_log_freq;
	replot();
}

std::vector<double*> FftDisplayPlot::getOrginal_data() {
    return y_original_data;
}

int64_t FftDisplayPlot::getYdata_size() {
    return y_data.size();
}

std::vector<double*> FftDisplayPlot::getRef_data() {
    return d_refYdata;
}

std::vector<double> FftDisplayPlot::getScaleFactor() {
    return y_scale_factor;
}

int64_t FftDisplayPlot::getNumPoints()
{
    return d_numPoints;
}

void FftDisplayPlot::plotData(const std::vector<double *> &pts,
		uint64_t num_points)
{
	uint64_t halfNumPoints = num_points / 2;
	bool numPointsChanged = false;
	bool samplRateChanged = false;
	bool magTypeChanged = false;

	// Update sample rate if required
	if (d_sampl_rate != d_preset_sampl_rate) {
		d_sampl_rate = d_preset_sampl_rate;
		d_start_frequency = 0;
		d_stop_frequency = d_sampl_rate / 2;
		samplRateChanged = true;

		Q_EMIT sampleRateUpdated(d_sampl_rate);
	}

	if (d_magType != d_presetMagType) {
		d_magType = d_presetMagType;
		magTypeChanged = true;
	}

	if (d_stop || halfNumPoints == 0)
		return;

	if (halfNumPoints != d_numPoints || d_firstInit) {
		d_firstInit = false;
		d_numPoints = halfNumPoints;
		numPointsChanged = true;

		Q_EMIT sampleCountUpdated(d_numPoints);

		if (x_data)
			delete []x_data;

		x_data = new double[halfNumPoints];

		for (unsigned int i = 0; i < d_nplots; i++) {
			if (y_data[i])
				delete[] y_data[i];
			if (y_original_data[i])
				delete[] y_original_data[i];

			y_data[i] = new double[halfNumPoints];
			y_original_data[i] = new double[halfNumPoints];

#if QWT_VERSION < 0x060000
			d_plot_curve[i]->setRawData(x_data,
					y_data[i], halfNumPoints);
#else
			d_plot_curve[i]->setRawSamples(x_data,
					y_data[i], halfNumPoints);
#endif
		}

		// Resize the average objects to the new number of points
		for (int i = 0; i < d_ch_avg_obj.size(); i++) {
			if (!d_ch_avg_obj[i])
				continue;

			uint size = d_ch_avg_obj[i]->dataWidth();
			if (size == halfNumPoints)
				continue;

			uint h = d_ch_avg_obj[i]->history();
			bool h_en = d_ch_avg_obj[i]->historyEnabled();
			d_ch_avg_obj[i] = getNewAvgObject(
				d_ch_average_type[i], halfNumPoints, h, h_en);
		}
	}

	// We store the received data before touching it
	for (unsigned int i = 0; i < d_nplots; i++) {
		memcpy(y_original_data[i], pts[i],
				halfNumPoints * sizeof(double));
	}

	// When the magnitude type changes, we reset the data that is
	// being stored in the average objects
	if (magTypeChanged) {
		resetAverageHistory();
	}

	averageDataAndComputeMagnitude(y_original_data, y_data, halfNumPoints);

	_resetXAxisPoints();

	if (numPointsChanged) {
		// When the number of points change but the start and stop freq
		// stay the same, we need to update the position of fixed markers
		for (int c = 0; c < d_nplots; c++) {
			for (int m = 0; m < d_markers[c].size(); m++) {
				auto marker = d_markers[c][m];
				if (!marker.data || marker.data->type != 0)
					continue;

				marker.data->bin = posAtFrequency(marker.data->x);
				marker.data->x = x_data[marker.data->bin];
			}
		}
	}
	if (samplRateChanged) {
		// When the sample rate changes, the frequency of each bin
		// chhanges. Markers need to be updated so that they point to
		// the same frequency as before.
		for (int c = 0; c < d_nplots; c++) {
			for (int m = 0; m < d_markers[c].size(); m++) {
				auto marker = d_markers[c][m];
				if (!marker.data || marker.data->type != 0)
					continue;

				if (marker.data->x > d_stop_frequency) {
					marker.data->bin = d_numPoints - 1;
				} else {
					marker.data->bin = posAtFrequency(
						marker.data->x);
				}

				marker.data->x = x_data[marker.data->bin];
			}
		}
	}

	detectMarkers();

	_editFirstPoint();
	replot();

	Q_EMIT newData();
}

void FftDisplayPlot::_editFirstPoint()
{
	// Set first point on the xAxis to 1 in order for it to
	// be plotted and make the y_data[i][0] values equal to the ones
	// of the next point to draw a straight line from the start of
	// the plot to the start of the sweep
	x_data[0] = d_logScaleEnabled;
	for (size_t i = 0; i < y_data.size(); ++i) {
		y_data[i][0] = y_data[i][1];
	}

}

void FftDisplayPlot::averageDataAndComputeMagnitude(std::vector<double *>
	in_data, std::vector<double *> out_data, uint64_t nb_points)
{
	std::vector<double *> source;

	if (d_buffer_idx == 0) {
		d_ps_avg.resize(d_nplots);
	}
	for (unsigned int i = 0; i < d_nplots; i++) {
		bool needs_dB_avg = false;

		d_current_avg_index[i] += 1;
		if (averageHistory(i) > 0) {
			d_current_avg_index[i] %= averageHistory(i);
		}
		Q_EMIT currentAverageIndex(i, d_current_avg_index[i]);

		switch (d_ch_average_type[i]) {
		case LINEAR_DB:
		case EXPONENTIAL_DB:
			needs_dB_avg = true;
		case SAMPLE:
			source = in_data;
			break;
		default: // For all the other averaging types do the averaging
			// before converting to dB
			d_ch_avg_obj[i]->pushNewData(in_data[i]);
			d_ch_avg_obj[i]->getAverage(out_data[i], nb_points);
			source = out_data;
			break;
		}


		if (d_buffer_idx == 0) {
			d_ps_avg[i].resize(nb_points);
		}
		for (int s = 0; s < nb_points; s++) {
			//dB Full-Scale
			switch (d_magType) {
			case DBFS:
				out_data[i][s] = 10 * log10((source[i][s] /
					(2048 * 2048)) /
					(nb_points * nb_points));
				break;
			case DBV:
				out_data[i][s] = 10 * log10(source[i][s]) +
					20 * log10(y_scale_factor[i]) -
					20 * log10(nb_points) -
					20 * log10(sqrt(2));
				break;
			case DBU:
				out_data[i][s] = 10 * log10(source[i][s]) +
					20 * log10(y_scale_factor[i]) -
					20 * log10(nb_points) -
					20 * log10(sqrt(2) * 0.77459667);
				break;
			case VPEAK:
				out_data[i][s] = sqrt(source[i][s]) *
					y_scale_factor[i] / nb_points;
				break;
			case VRMS:
				/* Another formula for this would be
				 * sqrt(2 * (sqrt(source[i][s]) * sqrt(source[i][s])) /
				 * (d_win_coefficient_sum * d_win_coefficient_sum));
				 * This are equivalent (the only difference is the moment
				 * when we apply the window compensation (before the FFT, or after.
				 * With the current version, this is applied before (in calcCoherentPowerGain)
				 */
				out_data[i][s] = sqrt(source[i][s]) *
					y_scale_factor[i] / sqrt(2) / nb_points;
				break;
			case VROOTHZ:
				auto ps_rms = sqrt(source[i][s]) * y_scale_factor[i] /  sqrt(2) / nb_points;
				d_ps_avg[i][s] = sqrt((d_ps_avg[i][s] * d_ps_avg[i][s]) + (ps_rms * ps_rms));

				if (d_buffer_idx == (d_nb_overlapping_avg - 1)) {
					d_ps_avg[i][s] = d_ps_avg[i][s] / sqrt(d_nb_overlapping_avg);
					auto ls_rms = d_ps_avg[i][s];
					auto enbw = d_sampl_rate * d_win_coefficient_sum_sqr[i] /
							(d_win_coefficient_sum[i] * d_win_coefficient_sum[i]);
					auto ls_d_rms = ls_rms / sqrt(enbw);
					out_data[i][s] = ls_d_rms;
				}
				break;
			};
		}

		if (needs_dB_avg) {
			d_ch_avg_obj[i]->pushNewData(out_data[i]);
			d_ch_avg_obj[i]->getAverage(out_data[i], nb_points);
		}
	}
	if (d_buffer_idx == (d_nb_overlapping_avg - 1)) {
		d_ps_avg.clear();
		d_buffer_idx = 0;
	} else {
		d_buffer_idx++;
	}
}

void FftDisplayPlot::_resetXAxisPoints()
{
	double fft_bin_size = (d_stop_frequency - d_start_frequency)
		/ static_cast<double>(d_numPoints);

	double freqValue = d_start_frequency;
	for(int64_t loc = 0; loc < d_numPoints; loc++) {
		x_data[loc] = freqValue;
		freqValue += fft_bin_size;
	}
}

int64_t FftDisplayPlot::posAtFrequency(double freq, int chIdx) const
{
	int64_t pos = 0;
	if (chIdx < d_nplots) {
		double fft_bin_size = (d_stop_frequency - d_start_frequency)
			/ static_cast<double>(d_numPoints);

		pos = qRound64((freq - d_start_frequency) / (fft_bin_size));
	} else {

		unsigned int num_points = d_plot_curve[chIdx]->data()->size();
		double stop_frequency = d_refXdata[chIdx - d_nplots][num_points - 1];
		double start_frequency = d_refYdata[chIdx - d_nplots][0];
		double fft_bin_size = (stop_frequency - start_frequency)
			/ static_cast<double>(num_points);

		pos = qRound64((freq - start_frequency) / (fft_bin_size));
	}

	return pos;
}

void FftDisplayPlot::customEvent(QEvent *e)
{
	if (e->type() == TimeUpdateEvent::Type()) {
		TimeUpdateEvent *ev = static_cast<TimeUpdateEvent *>(e);

		this->plotData(ev->getTimeDomainPoints(),
				ev->getNumTimeDomainDataPoints());
	}
}

bool FftDisplayPlot::getLogScale() const
{
	return d_logScaleEnabled;
}

void FftDisplayPlot::setSampleRate(double sr, double units,
	const std::string &strunits)
{
	d_start_frequency = 0;
	d_stop_frequency = sr / 2;
	d_sampl_rate = sr;
	d_preset_sampl_rate = sr;

	_resetXAxisPoints();
}

double FftDisplayPlot::sampleRate()
{
	return d_sampl_rate;
}

void FftDisplayPlot::presetSampleRate(double sr)
{
	d_preset_sampl_rate = sr;
}

FftDisplayPlot::AverageType FftDisplayPlot::averageType(uint chIdx) const
{
	if (chIdx < d_ch_average_type.size())
		return d_ch_average_type[chIdx];

	return SAMPLE;
}

uint FftDisplayPlot::averageHistory(uint chIdx) const
{
	uint history = 0;

	if (chIdx < d_ch_average_type.size())
		if (d_ch_avg_obj[chIdx])
			history = d_ch_avg_obj[chIdx]->history();

	return history;
}

void FftDisplayPlot::setAverage(uint chIdx, enum AverageType avg_type,
	uint history, bool history_en)
{
	if (chIdx >= d_ch_average_type.size()) {
		return;
	}


	if (d_ch_avg_obj[chIdx] && (history != d_ch_avg_obj[chIdx]->history())
			&& (history_en == d_ch_avg_obj[chIdx]->historyEnabled())) {
		d_ch_avg_obj[chIdx]->setHistory(history);
	} else {
		d_ch_average_type[chIdx] = avg_type;
		d_ch_avg_obj[chIdx] = getNewAvgObject(avg_type, d_numPoints, history, history_en);
		d_current_avg_index[chIdx] = 0;
		Q_EMIT currentAverageIndex(chIdx, d_current_avg_index[chIdx]);
	}
}

void FftDisplayPlot::resetAverageHistory()
{
	for (int i = 0; i < d_ch_avg_obj.size(); i++)
		if (d_ch_avg_obj[i])
			d_ch_avg_obj[i]->reset();

	for (int i = 0; i < d_current_avg_index.size(); i++) {
		d_current_avg_index[i] = 0;
		Q_EMIT currentAverageIndex(i, d_current_avg_index[i]);
	}
}

FftDisplayPlot::average_sptr FftDisplayPlot::getNewAvgObject(
	enum AverageType avg_type, uint data_width, uint history, bool history_en)
{
	switch (avg_type) {
		case SAMPLE:
			return nullptr;

		case PEAK_HOLD:
			return boost::make_shared<PeakHold>(data_width,
				history);
		case PEAK_HOLD_CONTINUOUS:
			return boost::make_shared<PeakHoldContinuous>(
				data_width, history);
		case MIN_HOLD:
			return boost::make_shared<MinHold>(data_width, history);
		case MIN_HOLD_CONTINUOUS:
			return boost::make_shared<MinHoldContinuous>(data_width,
				history);
		case LINEAR_RMS:
		if (history_en) {
			return boost::make_shared<LinearRMS>(data_width,
				history);
		} else {
			return boost::make_shared<LinearRMSOne>(data_width,
				history);
		}
		case LINEAR_DB:
		if (history_en) {
			return boost::make_shared<LinearAverage>(data_width,
				history);
		} else {
			return boost::make_shared<LinearAverageOne>(data_width,
				history);
		}
		case EXPONENTIAL_RMS:
			return boost::make_shared<ExponentialAverage>(
				data_width, history);
		case EXPONENTIAL_DB:
			return boost::make_shared<ExponentialAverage>(
				data_width, history);
		default:
			return nullptr;
	}
}

QString FftDisplayPlot::leftVerAxisUnit() const { return d_yAxisUnit; }

void FftDisplayPlot::setLeftVertAxisUnit(const QString& unit)
{
	if (d_yAxisUnit != unit) {
			d_yAxisUnit = unit;

		auto scale_draw = dynamic_cast<OscScaleDraw *>(
			axisScaleDraw(QwtAxis::YLeft));
		if (scale_draw) {
			scale_draw->setUnitType(unit);
		}
	}
}

QString FftDisplayPlot::btmHorAxisUnit() const { return d_xAxisUnit; }

void FftDisplayPlot::setBtmHorAxisUnit(const QString &unit)
{
	if (d_xAxisUnit != unit) {
		d_xAxisUnit = unit;

		auto scale_draw = dynamic_cast<OscScaleDraw*>(axisScaleDraw(QwtAxis::XBottom));
		if (scale_draw)
			scale_draw->setUnitType(unit);
	}
}

void FftDisplayPlot::findPeaks(int chn)
{
	QList<std::shared_ptr<struct marker_data>>& markers = d_peaks[chn];
	QList<std::shared_ptr<struct marker_data>>& f_sort_mrks = d_freq_asc_sorted_peaks[chn];
	int marker_count = markers.size();
	int maxX[marker_count + 1];
	float maxY[marker_count + 1];
	double *x = nullptr;
	double *y = nullptr;
	unsigned int num_points = 0;
	if (chn < d_nplots) {
		x = x_data;
		y = y_data[chn];
		num_points = d_numPoints;
	} else {
		x = d_refXdata[chn - d_nplots];
		y = d_refYdata[chn - d_nplots];
		num_points = d_plot_curve[chn]->data()->size();
	}

	if (!x || !y) {
		return;
	}

	for (int i = 0; i <= marker_count; i++) {
		maxX[i] = 0;
		maxY[i] = -200.0;
	}

	maxY[0] = y[0];

	auto start = 3;
	auto stop = num_points;

	if(m_visiblePeakSearch)
	{
		auto coef  = num_points/d_stop_frequency;
		if (m_sweepStart * coef > 0) {
			start = m_sweepStart * coef;
		}
		stop = m_sweepStop * coef;
		maxY[0] = y[start];
	}

	for (int i = start; i < stop; i++) {
		for (int j = 0; j < marker_count; j++ ) {
			if (i < 2) {
				continue;
			}
			if  ((y[i - 1] > maxY[j]) &&
					((!((y[i - 2] > y[i - 1]) &&
					 (y[i - 1] > y[i]))) &&
					 (!((y[i - 2] < y[i - 1]) &&
					 (y[i - 1] < y[i]))))) {

				for (int k = marker_count; k > j; k--) {
					maxY[k] = maxY[k - 1];
					maxX[k] = maxX[k - 1];
				}
				maxY[j] = y[i - 1];
				maxX[j] = i - 1;
				break;
			}
		}
	}

	for (int i = 0; i < marker_count; i++) {
		markers[i]->x = x[maxX[i]];
		markers[i]->y = y[maxX[i]];
		markers[i]->bin = maxX[i];
	}

	for (int i = 0; i < markers.size(); i++) {
		f_sort_mrks[i] = markers[i];
	}
	std::sort(f_sort_mrks.begin(), f_sort_mrks.end(),
		[](const std::shared_ptr<struct marker_data> m1,
			const std::shared_ptr<struct marker_data> m2) -> bool
			{
				return m1->x < m2->x;
			});

	updateMarkersUi();
}

void FftDisplayPlot::updateMarkerUi(uint chIdx, uint mkIdx)
{
	auto marker = d_markers[chIdx][mkIdx];

	// update marker ony if active
	if (marker.data && marker.data->update_ui) {
		marker.ui->setValue(marker.data->x, marker.data->y);
	}
}

void FftDisplayPlot::updateMarkersUi()
{
	for (int c = 0; c < d_nplots; c++) {
		for (int i = 0; i < d_markers[c].size(); i++) {
			updateMarkerUi(c, i);
		}
	}
}

void FftDisplayPlot::add_marker(int chn)
{
	QString markerName = QString("M%1").arg(d_markers[chn].size() + 1);

	// Data marker
	auto marker_data = std::make_shared<struct marker_data>();
	marker_data->type = 0; // Fixed marker
	marker_data->x = 0;
	marker_data->y = 0;
	marker_data->bin = 0;
	marker_data->update_ui = true;

	// GUI marker
	auto gui_marker = std::make_shared<SpectrumMarker>(markerName);
	QColor marker_color = d_markerColors[chn % d_markerColors.size()];
	QwtSymbol *symbol = new QwtSymbol(
		QwtSymbol::Diamond, QColor(237, 28, 36),
		QPen(marker_color, 2, Qt::SolidLine),
		QSize(18, 18));
	symbol->setSize(18, 18);

	gui_marker->setSymbol(symbol);
	gui_marker->setLabel(gui_marker->title());
	gui_marker->setLabelAlignment(Qt::AlignTop);
	gui_marker->setDefaultColor(QColor(237, 28, 36));
	gui_marker->attach(this);

	QwtText mrk_lbl = gui_marker->label();
	mrk_lbl.setColor(Qt::white);

	QFont lbl_font = mrk_lbl.font();
	lbl_font.setBold(true);
	lbl_font.setPixelSize(11);

	mrk_lbl.setFont(lbl_font);
	gui_marker->setLabel(mrk_lbl);

	struct marker marker;
	marker.data = marker_data;
	marker.ui = gui_marker;
	d_markers[chn].push_back(marker);
}

void FftDisplayPlot::remove_marker(int chn, int which)
{
	if (which < d_markers[chn].size()) {
		d_markers[chn][which].ui->detach();
		d_markers[chn].removeAt(which);
	}

}

void FftDisplayPlot::marker_set_pos_source(uint chIdx, uint mkIdx,
			std::shared_ptr<struct marker_data> &source_sptr)
{
	d_markers[chIdx][mkIdx].data = source_sptr;
	if (d_emitNewMkrData)
		Q_EMIT newMarkerData();
}

void FftDisplayPlot::calculate_fixed_markers(int chn)
{
	for (int i = 0; i < d_markers[chn].size(); i++) {

		double *ydata = nullptr, *xdata = nullptr;
		unsigned int index = -1;
		if (chn < d_nplots) {
			ydata = y_data[chn];
			xdata = x_data;
		} else {
			ydata = d_refYdata[chn - d_nplots];
			xdata = d_refXdata[chn - d_nplots];
		}

		// update active markers only
		auto marker = d_markers[chn][i];
		if (marker.data && marker.data->type == 0) {
			marker.data->y = ydata[marker.data->bin];
			marker.data->x = xdata[marker.data->bin];
		}
	}
}

uint FftDisplayPlot::peakCount(uint chIdx) const
{
	if (chIdx >= d_peaks.size())
		return 0;

	return d_peaks[chIdx].size();
}

void FftDisplayPlot::setPeakCount(uint chIdx, uint count)
{
	if (chIdx >= d_peaks.size())
		return;

	if (d_peaks[chIdx].size() == count)
		return;


	d_peaks[chIdx].clear();
	d_freq_asc_sorted_peaks[chIdx].clear();

	for (uint i = 0; i < count; i++) {
		auto data_marker_sp = std::make_shared<struct marker_data>();
		data_marker_sp->type = 1; // Peak marker
		data_marker_sp->update_ui = true;
		d_peaks[chIdx].push_back(data_marker_sp);
		d_freq_asc_sorted_peaks[chIdx].push_back(data_marker_sp);
	}
}

uint FftDisplayPlot::markerCount(uint chIdx) const
{
	return d_markers[chIdx].size();
}

void FftDisplayPlot::setMarkerCount(uint chIdx, uint count)
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return;
	}

	if (d_markers[chIdx].size() == count) {
		return;
	}

	while (d_markers[chIdx].size()) {
		remove_marker(chIdx, 0);
	}

	for (uint i = 0; i < count; i++) {
		add_marker(chIdx);
	}
}

bool FftDisplayPlot::markerEnabled(uint chIdx, uint mkIdx) const
{
	return !!d_markers[chIdx][mkIdx].data;
}


bool FftDisplayPlot:: markerVisible(uint chIdx, uint mkIdx) const
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return false;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return false;
	}

	return d_markers[chIdx][mkIdx].ui->isVisible();
}

void FftDisplayPlot::setMarkerVisible(uint chIdx, uint mkIdx, bool en)
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return;
	}

	d_markers[chIdx][mkIdx].ui->setVisible(en);
}

void FftDisplayPlot:: setMarkerEnabled(uint chIdx, uint mkIdx, bool en)
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return;
	}

	if (en) {
		auto data_sp = std::make_shared<struct marker_data>();
		data_sp->x = 0;
		data_sp->y = axisScaleDiv(QwtAxis::YLeft).lowerBound();
		data_sp->bin = 0;
		data_sp->update_ui = true;

		d_markers[chIdx][mkIdx].data = data_sp;
		d_markers[chIdx][mkIdx].ui->setValue(data_sp->x, data_sp->y);

		d_mrkCtrl->registerMarker(d_markers[chIdx][mkIdx].ui);

	} else {
		d_markers[chIdx][mkIdx].data = nullptr;
		d_mrkCtrl->unRegisterMarker(d_markers[chIdx][mkIdx].ui);
	}

	d_markers[chIdx][mkIdx].ui->setVisible(en);

	int en_markers = 0;
	for (int c = 0;  c < d_nplots; c++)
		for (int m = 0; m < d_markers[c].size(); m++)
			if (!!d_markers[c][m].data)
				en_markers++;
	d_emitNewMkrData = (en_markers > 0);
}

double FftDisplayPlot::markerFrequency(uint chIdx, uint mkIdx) const
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return 0;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return 0;
	}

	return d_markers[chIdx][mkIdx].data->x;
}

double FftDisplayPlot::markerMagnitude(uint chIdx, uint mkIdx) const
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return 0;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return 0;
	}

	return d_markers[chIdx][mkIdx].data->y;
}

void FftDisplayPlot::setMarkerAtFreq(uint chIdx, uint mkIdx, double freq)
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return;
	}

	int64_t pos = posAtFrequency(freq, chIdx);
	if ((pos < 0 || pos >= d_numPoints) && chIdx < d_nplots) {
		qDebug() << "Invalid frenquency!";
		return;
	}

	unsigned int index = -1;
	double *ydata = nullptr, *xdata = nullptr;
	if (chIdx < d_nplots) {
		ydata = y_data[chIdx];
		xdata = x_data;
		index = chIdx;
	} else {
		ydata = d_refYdata[chIdx - d_nplots];
		xdata = d_refXdata[chIdx - d_nplots];
		index = chIdx - d_nplots;
	}

	double y;
	if (ydata) {
		y = ydata[pos];
	} else {
		y = axisScaleDiv(QwtAxis::YLeft).lowerBound();
	}

	if (d_markers[chIdx][mkIdx].data->type != 0) {
		auto marker_data = std::make_shared<struct marker_data>();
		marker_data->type = 0; // Fixed marker
		marker_data->x = xdata[pos];
		marker_data->y = y;
		marker_data->bin = pos;
		marker_data->update_ui = d_markers[chIdx][mkIdx].data->update_ui;
		marker_set_pos_source(chIdx, mkIdx, marker_data);
	} else {
		d_markers[chIdx][mkIdx].data->x = xdata[pos];
		d_markers[chIdx][mkIdx].data->y = y;
		d_markers[chIdx][mkIdx].data->bin = pos;
	}
}

void FftDisplayPlot::marker_to_max_peak(uint chIdx, uint mkIdx)
{
	d_markers[chIdx][mkIdx].data = d_peaks[chIdx][0];

	if (d_emitNewMkrData)
		Q_EMIT newMarkerData();
}

void FftDisplayPlot::marker_to_next_higher_freq_peak(uint chIdx, uint mkIdx)
{
	auto peaks = d_freq_asc_sorted_peaks[chIdx];
	double freq = d_markers[chIdx][mkIdx].ui->value().x();
	int pos = -1;

	// find the first peak with the freq higher that marker freq pos
	for (int i = 0; i < peaks.size(); i++) {
		if (peaks[i]->x > freq) {
			pos = i;
			break;
		}
	}

	if (pos < 0)
		return;

	marker_set_pos_source(chIdx, mkIdx, peaks[pos]);
}

void FftDisplayPlot::marker_to_next_lower_freq_peak(uint chIdx, uint mkIdx)
{
	auto peaks = d_freq_asc_sorted_peaks[chIdx];
	double freq = d_markers[chIdx][mkIdx].ui->value().x();
	int pos = -1;

	// find the first peak with the freq lower that marker freq pos
	for (int i = peaks.size() - 1; i >= 0; i--) {
		if (peaks[i]->x < freq) {
			pos = i;
			break;
		}
	}

	if (pos < 0)
		return;

	marker_set_pos_source(chIdx, mkIdx, peaks[pos]);
}

void FftDisplayPlot::marker_to_next_higher_mag_peak(uint chIdx, uint mkIdx)
{
	auto peaks = d_peaks[chIdx];
	double mag = d_markers[chIdx][mkIdx].ui->value().y();
	int pos = -1;

	// find the first peak with the magnitude higher than the current marker
	for (int i = peaks.size() - 1; i >= 0; i--) {
		if (peaks[i]->y > mag) {
			pos = i;
			break;
		}
	}

	if (pos < 0)
		return;

	marker_set_pos_source(chIdx, mkIdx, peaks[pos]);
}

void FftDisplayPlot::setStartStop(double start, double stop)
{
	m_sweepStart = start;
	m_sweepStop = stop;
	auto div = axisScaleDiv(QwtAxis::XBottom);
	setXaxisNumDiv((div.ticks(2)).size() - 1);
	setXaxisMajorTicksPos(div.ticks(2));
}

void FftDisplayPlot::setVisiblePeakSearch(bool enabled)
{
	m_visiblePeakSearch = enabled;
}

void FftDisplayPlot::marker_to_next_lower_mag_peak(uint chIdx, uint mkIdx)
{
	auto peaks = d_peaks[chIdx];
	double mag = d_markers[chIdx][mkIdx].ui->value().y();
	int pos = -1;

	// find the first peak with the magnitude lower than the current marker
	for (int i = 0; i < peaks.size(); i++) {
		if (peaks[i]->y < mag) {
			pos = i;
			break;
		}
	}

	if (pos < 0)
		return;

	marker_set_pos_source(chIdx, mkIdx, peaks[pos]);
}

int FftDisplayPlot::getMarkerPos(const QList<marker>& marker_list,
	std::shared_ptr<SpectrumMarker> &marker) const
{
	int pos = -1;

	auto it = std::find_if(marker_list.begin(), marker_list.end(),
		[&](const struct marker &mrk) {
			return mrk.ui == marker;
		});
	if (it != marker_list.end()) {
		pos = it - marker_list.begin();
	}

	return pos;
}

void FftDisplayPlot::onMrkCtrlMarkerSelected(std::shared_ptr<SpectrumMarker>
	&marker)
{
	for (uint i = 0; i < d_nplots; i++) {
		for (uint j = 0; j < d_markers[i].size(); j++) {
			if (d_markers[i][j].ui == marker) {
				marker->setSelected(true);
				Q_EMIT markerSelected(i, j);
			} else {
				d_markers[i][j].ui->setSelected(false);
			}
		}
	}
}

void FftDisplayPlot::onMrkCtrlMarkerPosChanged(std::shared_ptr<SpectrumMarker> &marker)
{
	int markerPos = 0;
	uint chn = -1;

	for (uint i = 0; i < d_nplots + n_ref_curves; i++) {
		markerPos = getMarkerPos(d_markers[i], marker);
		if (markerPos >= 0) {
			chn = i;
			break;
		}
	}

	if (markerPos < 0) {
		qDebug() << "unknown marker in marker controller";
		return;
	}

	int bin = posAtFrequency(marker->value().x(), chn);
	if (bin < 0) {
		qDebug() << "bin should not be negative";
		return;
	}
	if (bin >= d_numPoints && chn < d_nplots) {
		bin = d_numPoints - 1;
	}

	auto marker_data = std::make_shared<struct marker_data>();

	unsigned int index = -1;
	double *ydata = nullptr, *xdata = nullptr;
	if (chn < d_nplots) {
		ydata = y_data[chn];
		xdata = x_data;
		index = chn;
	} else {
		ydata = d_refYdata[chn - d_nplots];
		xdata = d_refXdata[chn - d_nplots];
		index = chn - d_nplots;
	}

	double y;
	if (ydata) {
		y = ydata[bin];
	} else {
		qDebug() << "problem";
		y = axisScaleDiv(QwtAxis::YLeft).upperBound();
	}

	marker_data->type = 0; // Fixed marker
	marker_data->x = xdata[bin];
	marker_data->y = y;
	marker_data->bin = bin;
	marker_data->update_ui = false;

	marker_set_pos_source(chn, markerPos, marker_data);
}

void FftDisplayPlot::onMrkCtrlMarkerReleased(std::shared_ptr<SpectrumMarker>
	&marker)
{
	int markerPos = -1;
	uint chn = -1;

	for (uint i = 0; i < d_nplots + n_ref_curves; i++) {
		markerPos = getMarkerPos(d_markers[i], marker);
		if (markerPos >= 0) {
			chn = i;
			break;
		}
	}

	if (markerPos < 0) {
		qDebug() << "unknown marker in marker controller";
		return;
	}

	d_markers[chn][markerPos].data->update_ui = true;
	updateMarkerUi(chn, markerPos);
	replot();
}

void FftDisplayPlot::selectMarker(uint chIdx, uint mkIdx)
{
	for (uint i = 0; i < d_nplots; i++) {
		for (uint j = 0; j < d_markers[i].size(); j++) {
			d_markers[i][j].ui->setSelected(false);
		}
	}
	d_markers[chIdx][mkIdx].ui->setSelected(true);

	replot();
}

int FftDisplayPlot::markerType(uint chIdx, uint mkIdx) const
{
	if (chIdx >= d_markers.size()) {
		qDebug() << "Invalid channel index!";
		return -1;
	}

	if (mkIdx >= d_markers[chIdx].size()) {
		qDebug() << "Invalid marker index";
		return -1;
	}

	return d_markers[chIdx][mkIdx].data->type;
}

double FftDisplayPlot::channelScaleFactor(int chIdx) const
{
	return y_scale_factor[chIdx];
}

void FftDisplayPlot::setScaleFactor(int chIdx, double scale)
{
	y_scale_factor[chIdx] = scale;
}

FftDisplayPlot::MagnitudeType FftDisplayPlot::magnitudeType() const
{
	return d_magType;
}

void FftDisplayPlot::setMagnitudeType(enum MagnitudeType type)
{
	d_presetMagType = type;
	d_buffer_idx = 0;
	d_ps_avg.clear();
}

void FftDisplayPlot::setNbOverlappingAverages(unsigned int nb_avg)
{
	d_buffer_idx = 0;
	d_ps_avg.clear();
	d_nb_overlapping_avg = nb_avg;
}

/*
 * This can be used to make the plot recalculate the spectrum magnitude
 */
void FftDisplayPlot::recalculateMagnitudes()
{
	// Check if at least one acquisition has been made
	for (unsigned int i = 0; i < d_nplots; i++) {
		if (!y_data[i])
			return;
	}

	if (d_presetMagType != d_magType) {
		d_magType = d_presetMagType;
		resetAverageHistory();
	}

	averageDataAndComputeMagnitude(y_original_data, y_data, d_numPoints);
	detectMarkers();

	Q_EMIT newData();
}

/*
 * Does all the work for detecting the enabled markers
 */
void FftDisplayPlot::detectMarkers()
{
	for (int i = 0; i < d_nplots + n_ref_curves; i++) {
		calculate_fixed_markers(i);
		findPeaks(i);

		if (d_emitNewMkrData)
			Q_EMIT newMarkerData();
	}
}
