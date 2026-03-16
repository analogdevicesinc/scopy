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
#include "plotnavigator.hpp"
#include "plotscales.h"

#include <pluginbase/preferences.h>
#include <plot_utils.hpp>

#include <qwt_scale_widget.h>
#include <cfloat>

using namespace scopy;

// =============================================================================
// WaterfallData
// =============================================================================

WaterfallData::WaterfallData()
	: QwtRasterData()
	, m_maxRows(0)
	, m_fftSize(0)
	, m_xInterval(0.0, 1.0)
	, m_zInterval(-120.0, 0.0)
{}

WaterfallData::~WaterfallData() {}

void WaterfallData::addFFTData(const double *data, size_t size)
{
	if(!data || size == 0)
		return;

	if(m_fftSize == 0)
		m_fftSize = size;

	if(size != m_fftSize) {
		m_data.clear();
		m_fftSize = size;
	}

	m_data.push_back(std::vector<double>(data, data + size));

	while(static_cast<int>(m_data.size()) > m_maxRows)
		m_data.pop_front();
}

void WaterfallData::reset() { m_data.clear(); }

void WaterfallData::setXInterval(double minFreq, double maxFreq) { m_xInterval = QwtInterval(minFreq, maxFreq); }

void WaterfallData::setZInterval(double minDb, double maxDb) { m_zInterval = QwtInterval(minDb, maxDb); }

void WaterfallData::setMaxRows(int rows)
{
	if(rows <= 0)
		return;
	m_maxRows = rows;
}

int WaterfallData::maxRows() const { return m_maxRows; }

int WaterfallData::rowCount() const { return static_cast<int>(m_data.size()); }

QwtInterval WaterfallData::interval(Qt::Axis axis) const
{
	switch(axis) {
	case Qt::XAxis:
		return m_xInterval;
	case Qt::YAxis:
		return QwtInterval(0.0, static_cast<double>(m_maxRows));
	case Qt::ZAxis:
		return m_zInterval;
	default:
		return QwtInterval();
	}
}

double WaterfallData::value(double x, double y) const
{
	if(m_data.empty() || m_fftSize == 0)
		return -DBL_MAX;

	const int nRows = static_cast<int>(m_data.size());

	// Y is a row index in [0, maxRows]. Newest rows are at the top (high y).
	const int row = static_cast<int>(y);
	const int dataRow = row - (m_maxRows - nRows);

	if(dataRow < 0 || dataRow >= nRows)
		return -DBL_MAX;

	const double xRange = m_xInterval.maxValue() - m_xInterval.minValue();
	if(xRange <= 0.0)
		return -DBL_MAX;

	const int bin =
		static_cast<int>((x - m_xInterval.minValue()) / xRange * static_cast<double>(m_fftSize - 1) + 0.5);

	if(bin < 0 || static_cast<size_t>(bin) >= m_fftSize)
		return -DBL_MAX;

	return m_data[static_cast<size_t>(dataRow)][static_cast<size_t>(bin)];
}

// =============================================================================
// WaterfallTimeFormatter
// =============================================================================

WaterfallTimeFormatter::WaterfallTimeFormatter(QObject *parent)
	: MetricPrefixFormatter(parent)
	, m_secsPerRow(1.0)
{
	setTrimZeroes(true);
	setTwoDecimalMode(false);
}

void WaterfallTimeFormatter::setSecsPerRow(double secs) { m_secsPerRow = secs; }

QString WaterfallTimeFormatter::format(double value, QString unitType, int precision) const
{
	return MetricPrefixFormatter::format(value * m_secsPerRow, unitType, precision);
}

// =============================================================================
// WaterfallPlotWidget
// =============================================================================

WaterfallPlotWidget::WaterfallPlotWidget(QWidget *parent)
	: PlotWidget(parent)
	, m_secsPerRow(1.0 / Preferences::GetInstance()->get("general_plot_target_fps").toDouble())
	, m_rowCount(0)
	, m_waterfallEnabled(false)
{
	navigator()->addAxis(xAxis());
	navigator()->setYAxesEn(false);

	m_data = new WaterfallData();
	m_data->setXInterval(xAxis()->min(), xAxis()->max());
	m_data->setZInterval(-120.0, 0.0);

	m_spectrogram = new QwtPlotSpectrogram();
	m_spectrogram->setData(m_data);
	m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
	m_spectrogram->setColorMap(new WaterfallColorMap());
	m_spectrogram->attach(plot());

	// Y axis: row indices [maxRows, 0] — inverted so newest (0) is at top.
	yAxis()->setInterval(m_data->maxRows(), 0);

	m_timeFormatter = new WaterfallTimeFormatter(this);
	m_timeFormatter->setSecsPerRow(m_secsPerRow);
	yAxis()->setFormatter(m_timeFormatter);
	yAxis()->setUnits("s");
	yAxis()->setUnitsVisible(true);

	xAxis()->setUnits("Hz");
	xAxis()->setUnitsVisible(true);

	setShowXAxisLabels(true);
	setShowYAxisLabels(true);
	showAxisLabels();

	scales()->setGridEn(false);
	scales()->setGraticuleEn(false);
}

WaterfallPlotWidget::~WaterfallPlotWidget() {}

void WaterfallPlotWidget::setWaterfallEnabled(bool enabled) { m_waterfallEnabled = enabled; }

void WaterfallPlotWidget::addFFTData(const double *data, size_t size)
{
	if(!m_waterfallEnabled)
		return;

	if(m_rowTimer.isValid()) {
		const double elapsed = m_rowTimer.elapsed() / 1000.0;
		m_rowTimer.restart();
		if(elapsed > 0.0) {
			// EMA: slow enough to track steady-state load, fast enough to react to config changes.
			static constexpr double alpha = 0.1;
			m_secsPerRow = alpha * elapsed + (1.0 - alpha) * m_secsPerRow;
			++m_rowCount;
			if(m_rowCount % 20 == 0) {
				m_timeFormatter->setSecsPerRow(m_secsPerRow);
				yAxis()->scaleDraw()->invalidateCache();
				plot()->axisWidget(yAxis()->axisId())->update();
			}
		}
	}

	m_data->addFFTData(data, size);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
	Q_EMIT newData();
}

void WaterfallPlotWidget::clearData()
{
	m_data->reset();
	m_rowTimer.invalidate();
	m_rowCount = 0;
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
	replot();
}

void WaterfallPlotWidget::setFrequencyRange(double startHz, double stopHz)
{
	m_data->setXInterval(startHz, stopHz);
	m_data->reset();
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

void WaterfallPlotWidget::setIntensityRange(double minDb, double maxDb)
{
	m_data->setZInterval(minDb, maxDb);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

void WaterfallPlotWidget::updateYAxis()
{
	m_rowCount = 0;
	m_rowTimer.start();
}

void WaterfallPlotWidget::setNumRows(int rows)
{
	if(rows <= 0)
		return;
	m_data->setMaxRows(rows);
	yAxis()->setInterval(rows, 0);
}

#include "moc_waterfallplotwidget.cpp"
