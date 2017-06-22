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

#include <qwt_symbol.h>
#include <boost/make_shared.hpp>

using namespace adiscope;

FftDisplayPlot::FftDisplayPlot(int nplots, QWidget *parent) :
	DisplayPlot(nplots, parent),
	d_start_frequency(0),
	d_stop_frequency(1000)
{
	for (unsigned int i = 0; i < nplots; i++) {
		auto plot = new QwtPlotCurve(QString("Data %1").arg(i));

		plot->setPen(QPen(d_CurveColors[i]));
		plot->attach(this);

		d_plot_curve.push_back(plot);
		y_data.push_back(nullptr);

		d_ch_average_type.push_back(AverageType::SAMPLE);

		d_num_markers.push_back(0);
		d_markers.push_back(QList<marker_data>());
		d_gui_markers.push_back(QList<SpectrumMarker *>());
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

	setMinXaxisDivision(1);     // A minimum division of 1 Hz
	setMaxXaxisDivision(5E6);   // A maximum division of 5 MHz
	setMinYaxisDivision(1E-3);  // A minimum division of 1 mdB
	setMaxYaxisDivision(100); // A maximum division of 100 dB
	setVertUnitsPerDiv(20);
	setVertOffset(-VertUnitsPerDiv() * 5);
}

FftDisplayPlot::~FftDisplayPlot()
{
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

void FftDisplayPlot::plotData(const std::vector<double *> pts,
		uint64_t num_points)
{
	uint64_t halfNumPoints = num_points / 2;

	if (d_stop || halfNumPoints == 0)
		return;

	if (halfNumPoints != d_numPoints) {
		d_numPoints = halfNumPoints;

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
			y_data[i][s] = 10 * log10(4 * (y_data[i][s] / (2048 * 2048)) /
				(halfNumPoints * halfNumPoints));
		}

		if (needs_dB_avg) {
			d_ch_avg_obj[i]->pushNewData(y_data[i]);
			d_ch_avg_obj[i]->getAverage(y_data[i], halfNumPoints);
		}
	}

	_resetXAxisPoints();

	for (int i = 0; i < d_nplots; i++) {
		findPeaks(i);
	}

	replot();
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

	_resetXAxisPoints();
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
	QList<struct marker_data>& markers = d_markers[chn];
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
		markers[i].x = x[maxX[i]];
		markers[i].y = y[maxX[i]];
		markers[i].bin = maxX[i];

		d_gui_markers[chn][i]->setValue(markers[i].x, markers[i].y);
	}
}

void FftDisplayPlot::add_marker(int chn)
{
	QString markerName = QString("M%1").arg(d_markers[chn].size());
	// Data Marker
	struct marker_data data_marker;
	data_marker.label = markerName;
	d_markers[chn].push_back(data_marker);

	// GUI Marker
	SpectrumMarker *gui_marker = new SpectrumMarker(markerName);
	QwtSymbol *symbol = new QwtSymbol(
		QwtSymbol::Diamond,QColor(255, 242, 0),
		QPen(QColor(237, 28, 36), 2, Qt::SolidLine),
		QSize(18, 18));
	symbol->setSize(18, 18);
	gui_marker->setSymbol(symbol);
	gui_marker->setLabel(gui_marker->title());
	gui_marker->setLabelAlignment(Qt::AlignTop);
	gui_marker->attach(this);

	QwtText mrk_lbl = gui_marker->label();
	mrk_lbl.setColor(Qt::white);

	QFont lbl_font = mrk_lbl.font();
	lbl_font.setBold(true);
	lbl_font.setPixelSize(11);

	mrk_lbl.setFont(lbl_font);
	gui_marker->setLabel(mrk_lbl);

	d_gui_markers[chn].push_back(gui_marker);
}

void FftDisplayPlot::remove_marker(int chn, int which)
{
	if (which < d_markers[chn].size()) {
		d_markers[chn].removeAt(which);
		d_gui_markers[chn][which]->detach();
		delete d_gui_markers[chn][which];
		d_gui_markers[chn].removeAt(which);
	}
}

uint FftDisplayPlot::peakCount(uint chIdx) const
{
	if (chIdx >= d_markers.size())
		return 0;

	return d_markers[chIdx].size();
}

void FftDisplayPlot::setPeakCount(uint chIdx, uint count)
{
	if (chIdx >= d_markers.size())
		return;

	if (d_markers[chIdx].size() == count)
		return;

	while (d_markers[chIdx].size()) {
		remove_marker(chIdx, 0);
	}

	for (uint i = 0; i < count; i++) {
		add_marker(chIdx);
	}
}

bool FftDisplayPlot::isPeakVisible(uint chIdx, uint peakIdx) const
{
	if (chIdx >= d_markers.size())
		return false;

	if (peakIdx >= d_markers[chIdx].size())
		return false;

	return d_gui_markers[chIdx][peakIdx]->isVisible();
}

void FftDisplayPlot::setPeakVisible(uint chIdx, uint peakIdx, bool on)
{
	if (chIdx >= d_markers.size())
		return;

	if (peakIdx >= d_markers[chIdx].size())
		return;

	d_gui_markers[chIdx][peakIdx]->setVisible(on);
}
