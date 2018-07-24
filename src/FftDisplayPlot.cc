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

#include "FftDisplayPlot.h"
#include "spectrumUpdateEvents.h"
#include "signal_generator.hpp"
#include "average.h"
#include "spectrum_marker.hpp"
#include "marker_controller.h"
#include "limitedplotzoomer.h"

#include <qwt_symbol.h>
#include <boost/make_shared.hpp>

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
    QwtDoublePoint dp = QwtPlotZoomer::invTransform(p);
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
	m_visiblePeakSearch(true)
{
	// TO DO: Add more colors
	d_markerColors << QColor(255, 242, 0) << QColor(210, 155, 210);
	d_zoomer.push_back(nullptr);

	for (unsigned int i = 0; i < nplots; i++) {
		auto plot = new QwtPlotCurve(QString("CH %1").arg(i + 1));

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
	}
	y_scale_factor.resize(nplots);
	d_ch_avg_obj.resize(nplots);

	m_sweepStart = 0;
	m_sweepStop = 1000;
	d_magType = MagnitudeType::DBFS;

	d_numPoints = 1024;
	x_data = new double[d_numPoints];

	dBFormatter.setTwoDecimalMode(false);
	freqFormatter.setTwoDecimalMode(true);

	OscScaleDraw *yScaleDraw = new OscScaleDraw(&dBFormatter, "");
	setAxisScaleDraw(QwtPlot::yLeft, yScaleDraw);
	yScaleDraw->setFloatPrecision(2);

	OscScaleDraw *xScaleDraw = new OscScaleDraw(&freqFormatter, "Hz");
	setAxisScaleDraw(QwtPlot::xBottom, xScaleDraw);
	xScaleDraw->setFloatPrecision(2);

	_resetXAxisPoints();

	d_mrkCtrl = new MarkerController(this);

	connect(d_mrkCtrl,
		SIGNAL(markerSelected(std::shared_ptr<SpectrumMarker>)),
		this,
		SLOT(onMrkCtrlMarkerSelected(std::shared_ptr<SpectrumMarker>))
	);
	connect(d_mrkCtrl,
		SIGNAL(markerPositionChanged(std::shared_ptr<SpectrumMarker>)),
		this,
		SLOT(onMrkCtrlMarkerPosChanged(std::shared_ptr<SpectrumMarker>))
	);
	connect(d_mrkCtrl,
		SIGNAL(markerReleased(std::shared_ptr<SpectrumMarker>)),
		this,
		SLOT(onMrkCtrlMarkerReleased(std::shared_ptr<SpectrumMarker>))
	);


	setMinXaxisDivision(1);     // A minimum division of 1 Hz
	setMaxXaxisDivision(5E6);   // A maximum division of 5 MHz
	setMinYaxisDivision(1E-3);  // A minimum division of 1 mdB
	setMaxYaxisDivision(100); // A maximum division of 100 dB
	setVertUnitsPerDiv(20);
	setVertOffset(-VertUnitsPerDiv() * 5);
}

FftDisplayPlot::~FftDisplayPlot()
{
	for (uint c = 0; c < d_nplots; c++) {
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
}

void FftDisplayPlot::replot()
{
	QwtPlot::replot();
}

void FftDisplayPlot::setZoomerEnabled()
{
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

void FftDisplayPlot::plotData(const std::vector<double *> pts,
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

	if (halfNumPoints != d_numPoints) {
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
			d_ch_avg_obj[i] = getNewAvgObject(
				d_ch_average_type[i], halfNumPoints, h);
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
	replot();

	Q_EMIT newData();
}

void FftDisplayPlot::averageDataAndComputeMagnitude(std::vector<double *>
	in_data, std::vector<double *> out_data, uint64_t nb_points)
{
	std::vector<double *> source;

	for (unsigned int i = 0; i < d_nplots; i++) {
		bool needs_dB_avg = false;

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
				out_data[i][s] = sqrt(source[i][s]) *
					y_scale_factor[i] / sqrt(2) / nb_points;
				break;
			};
		}

		if (needs_dB_avg) {
			d_ch_avg_obj[i]->pushNewData(out_data[i]);
			d_ch_avg_obj[i]->getAverage(out_data[i], nb_points);
		}
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

int64_t FftDisplayPlot:: posAtFrequency(double freq) const
{
	int64_t pos;
	double fft_bin_size = (d_stop_frequency - d_start_frequency)
		/ static_cast<double>(d_numPoints);

	pos = qRound64((freq - d_start_frequency) / (fft_bin_size));

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
	uint history)
{
	if (chIdx >= d_ch_average_type.size()) {
		return;
	}

	d_ch_average_type[chIdx] = avg_type;
	d_ch_avg_obj[chIdx] = getNewAvgObject(avg_type, d_numPoints, history);
}

void FftDisplayPlot::resetAverageHistory()
{
	for (int i = 0; i < d_ch_avg_obj.size(); i++)
		if (d_ch_avg_obj[i])
			d_ch_avg_obj[i]->reset();
}

FftDisplayPlot::average_sptr FftDisplayPlot::getNewAvgObject(
	enum AverageType avg_type, uint data_width, uint history)
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
			return boost::make_shared<LinearAverage>(data_width,
				history);
		case LINEAR_DB:
			return boost::make_shared<LinearAverage>(data_width,
				history);
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

QString FftDisplayPlot::leftVerAxisUnit() const
{
	QString unit;
	auto scale_draw = dynamic_cast<const OscScaleDraw *>(
		axisScaleDraw(QwtPlot::yLeft));
	if (scale_draw)
		unit = scale_draw->getUnitType();

	return unit;
}

void FftDisplayPlot::setLeftVertAxisUnit(const QString& unit)
{
	auto scale_draw = dynamic_cast<OscScaleDraw *>(
		axisScaleDraw(QwtPlot::yLeft));
	if (scale_draw)
		scale_draw->setUnitType(unit);
}

void FftDisplayPlot::findPeaks(int chn)
{
	QList<std::shared_ptr<struct marker_data>>& markers = d_peaks[chn];
	QList<std::shared_ptr<struct marker_data>>& f_sort_mrks = d_freq_asc_sorted_peaks[chn];
	int marker_count = markers.size();
	int maxX[marker_count + 1];
	float maxY[marker_count + 1];
	double *x = x_data;
	double *y = y_data[chn];

	for (int i = 0; i <= marker_count; i++) {
		maxX[i] = 0;
		maxY[i] = -200.0;
	}

	maxY[0] = y[0];

	auto start = 3;
	auto stop = d_numPoints;

	if(m_visiblePeakSearch)
	{
		auto coef  = d_numPoints/d_stop_frequency;
		if (m_sweepStart * coef > 0) {
			start = m_sweepStart * coef;
		}
		stop = m_sweepStop * coef;
		maxY[0] = y[start];
	}

	for (int i = start; i < stop; i++) {
		for (int j = 0; j < marker_count; j++ ) {
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
			std::shared_ptr<struct marker_data> source_sptr)
{
	d_markers[chIdx][mkIdx].data = source_sptr;
	if (d_emitNewMkrData)
		Q_EMIT newMarkerData();
}

void FftDisplayPlot::calculate_fixed_markers(int chn)
{
	for (int i = 0; i < d_markers[chn].size(); i++) {
		// update active markers only
		auto marker = d_markers[chn][i];
		if (marker.data && marker.data->type == 0) {
			marker.data->y = y_data[chn][marker.data->bin];
			marker.data->x = x_data[marker.data->bin];
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

void FftDisplayPlot::setMarkerEnabled(uint chIdx, uint mkIdx, bool en)
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
		data_sp->y = axisScaleDiv(QwtPlot::yLeft).lowerBound();
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

	int64_t pos = posAtFrequency(freq);
	if (pos < 0 || pos >= d_numPoints) {
		qDebug() << "Invalid frenquency!";
		return;
	}

	double y;
	if (y_data[chIdx]) {
		y = y_data[chIdx][pos];
	} else {
		y = axisScaleDiv(QwtPlot::yLeft).lowerBound();
	}

	if (d_markers[chIdx][mkIdx].data->type != 0) {
		auto marker_data = std::make_shared<struct marker_data>();
		marker_data->type = 0; // Fixed marker
		marker_data->x = x_data[pos];
		marker_data->y = y;
		marker_data->bin = pos;
		marker_data->update_ui = d_markers[chIdx][mkIdx].data->update_ui;
		marker_set_pos_source(chIdx, mkIdx, marker_data);
	} else {
		d_markers[chIdx][mkIdx].data->x = x_data[pos];
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
	std::shared_ptr<SpectrumMarker> marker) const
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
	marker)
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

void FftDisplayPlot::onMrkCtrlMarkerPosChanged(std::shared_ptr<SpectrumMarker>
	marker)
{
	int markerPos = 0;
	uint chn = -1;

	for (uint i = 0; i < d_nplots; i++) {
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

	int bin = posAtFrequency(marker->value().x());
	if (bin < 0) {
		qDebug() << "bin should not be negative";
		return;
	}
	if (bin >= d_numPoints) {
		bin = d_numPoints - 1;
	}

	auto marker_data = std::make_shared<struct marker_data>();

	double y;
	if (y_data[chn]) {
		y = y_data[chn][bin];
	} else {
		y = axisScaleDiv(QwtPlot::yLeft).lowerBound();
	}

	marker_data->type = 0; // Fixed marker
	marker_data->x = x_data[bin];
	marker_data->y = y;
	marker_data->bin = bin;
	marker_data->update_ui = false;

	marker_set_pos_source(chn, markerPos, marker_data);
}

void FftDisplayPlot::onMrkCtrlMarkerReleased(std::shared_ptr<SpectrumMarker>
	marker)
{
	int markerPos = -1;
	uint chn = -1;

	for (uint i = 0; i < d_nplots; i++) {
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
	for (int i = 0; i < d_nplots; i++) {
		calculate_fixed_markers(i);
		findPeaks(i);

		if (d_emitNewMkrData)
			Q_EMIT newMarkerData();
	}
}
