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

#include <qwt_symbol.h>
#include <boost/make_shared.hpp>

using namespace adiscope;

class FftDisplayZoomer: public QwtPlotZoomer
{
public:
#if QWT_VERSION < 0x060100
  FftDisplayZoomer(QwtPlotCanvas* canvas)
#else /* QWT_VERSION < 0x060100 */
  FftDisplayZoomer(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
    : QwtPlotZoomer(canvas)
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
	d_mrkCtrl(nullptr),
	d_emitNewMkrData(true)
{
	// TO DO: Add more colors
	d_markerColors << QColor(255, 242, 0) << QColor(210, 155, 210);

	d_zoomer = nullptr;

	for (unsigned int i = 0; i < nplots; i++) {
		auto plot = new QwtPlotCurve(QString("Data %1").arg(i));

		plot->setPen(QPen(d_CurveColors[i]));
		plot->attach(this);

		d_plot_curve.push_back(plot);
		y_data.push_back(nullptr);

		d_ch_average_type.push_back(AverageType::SAMPLE);

		d_num_markers.push_back(0);
		d_markers.push_back(QList<marker>());
		d_peaks.push_back(
			QList<std::shared_ptr<marker_data>>());
		d_freq_asc_sorted_peaks.push_back(
			QList<std::shared_ptr<marker_data>>());
	}
	d_ch_avg_obj.resize(nplots);

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
	}
}

void FftDisplayPlot::replot()
{
	QwtPlot::replot();
}

void FftDisplayPlot::setZoomerEnabled()
{
        if(!d_zoomer) {
                d_zoomer = new FftDisplayZoomer(canvas());

                QFont font("DejaVu Sans", 10, 75);
                d_zoomer->setTrackerFont(font);

#if QWT_VERSION < 0x060000
                d_zoomer->setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection);
#endif

                d_zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                                          Qt::RightButton, Qt::ControlModifier);
                d_zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                                          Qt::RightButton);

                const QColor c("#999999");
                d_zoomer->setRubberBandPen(c);
                d_zoomer->setTrackerPen(c);
        }
}

void FftDisplayPlot::plotData(const std::vector<double *> pts,
		uint64_t num_points)
{
	uint64_t halfNumPoints = num_points / 2;
	bool numPointsChanged = false;
	bool samplRateChanged = false;

	// Update sample rate if required
	if (d_sampl_rate != d_preset_sampl_rate) {
		d_sampl_rate = d_preset_sampl_rate;
		d_start_frequency = 0;
		d_stop_frequency = d_sampl_rate / 2;
		samplRateChanged = true;

		Q_EMIT sampleRateUpdated(d_sampl_rate);
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

			y_data[i] = new double[halfNumPoints];

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

	for (unsigned int i = 0; i < d_nplots; i++) {
		bool needs_dB_avg = false;

		switch (d_ch_average_type[i]) {
		case LINEAR_DB:
			needs_dB_avg = true;
		case EXPONENTIAL_DB:
			needs_dB_avg = true;
		case SAMPLE:
			memcpy(y_data[i], pts[i],
				halfNumPoints * sizeof(double));
			break;
		default:
			d_ch_avg_obj[i]->pushNewData(pts[i]);
			d_ch_avg_obj[i]->getAverage(y_data[i], halfNumPoints);
			break;
		}

		for (int s = 0; s < halfNumPoints; s++) {
			//dB Full-Scale
			y_data[i][s] = 10 * log10((y_data[i][s] / (2048 * 2048)) /
				(halfNumPoints * halfNumPoints));
		}

		if (needs_dB_avg) {
			d_ch_avg_obj[i]->pushNewData(y_data[i]);
			d_ch_avg_obj[i]->getAverage(y_data[i], halfNumPoints);
		}
	}

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

	for (int i = 0; i < d_nplots; i++) {
		calculate_fixed_markers(i);
		findPeaks(i);

		if (d_emitNewMkrData)
			Q_EMIT newMarkerData();
	}

	replot();

	Q_EMIT newData();
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
	for (int i = 3; i < d_numPoints; i++) {
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
		QwtSymbol::Diamond, marker_color,
		QPen(QColor(237, 28, 36), 2, Qt::SolidLine),
		QSize(18, 18));
	symbol->setSize(18, 18);
	gui_marker->setSymbol(symbol);
	gui_marker->setLabel(gui_marker->title());
	gui_marker->setLabelAlignment(Qt::AlignTop);
	gui_marker->setDefaultColor(marker_color);
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

double FftDisplayPlot::markerMagnutide(uint chIdx, uint mkIdx) const
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
	int markerPos;
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
	int markerPos;
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
