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

#include <cmath>
#include <pluginbase/preferences.h>
#include <plot_utils.hpp>

#include <qwt_scale_widget.h>
#include <cfloat>
#include <vector>

using namespace scopy;

// =============================================================================
// WaterfallData
// =============================================================================

WaterfallData::WaterfallData()
	: QwtRasterData()
	, m_maxRows(0)
	, m_fftSize(0)
	, m_antialiasing(true)
	, m_xInterval(0.0, 1.0)
	, m_zInterval(-120.0, 0.0)
{}

WaterfallData::~WaterfallData() {}

void WaterfallData::addFFTData(const float *data, size_t size)
{
	if(!data || size == 0)
		return;

	if(m_fftSize == 0)
		m_fftSize = size;

	if(size != m_fftSize) {
		m_data.clear();
		m_fftSize = size;
	}

	m_data.push_back(std::vector<float>(data, data + size));

	while(static_cast<int>(m_data.size()) > m_maxRows)
		m_data.pop_front();
}

void WaterfallData::setSnapshot(std::vector<QVector<float>> rows)
{
	// rows[0]=newest, rows.back()=oldest (SampleBuffer convention).
	// m_data stores oldest-first (push_back = newest), so we reverse.
	m_data.clear();
	if(rows.empty()) {
		m_fftSize = 0;
		return;
	}
	m_fftSize = static_cast<size_t>(rows[0].size());
	for(int i = static_cast<int>(rows.size()) - 1; i >= 0; --i) {
		m_data.push_back(std::vector<float>(rows[i].begin(), rows[i].end()));
	}
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

void WaterfallData::setAntialiasing(bool enabled) { m_antialiasing = enabled; }

QwtInterval WaterfallData::interval(Qt::Axis axis) const
{
	switch(axis) {
	case Qt::XAxis:
		return m_xInterval;
	case Qt::YAxis:
		return QwtInterval(static_cast<double>(m_maxRows), 0.0);
	case Qt::ZAxis:
		return m_zInterval;
	default:
		return QwtInterval();
	}
}

double WaterfallData::value(double x, double y) const
{
	// yes, this actually happens if plot width or height is 0
	if(!std::isfinite(x) || !std::isfinite(y))
		return -DBL_MAX;

	if(m_data.empty() || m_fftSize == 0)
		return -DBL_MAX;

	const int nRows = static_cast<int>(m_data.size());

	const double dataRowF = (m_maxRows - 1.0 - y) - (m_maxRows - nRows);

	if(dataRowF < 0.0 || dataRowF >= static_cast<double>(nRows))
		return -DBL_MAX;

	const double xRange = m_xInterval.maxValue() - m_xInterval.minValue();
	if(xRange <= 0.0)
		return -DBL_MAX;

	const double binF = (x - m_xInterval.minValue()) / xRange * static_cast<double>(m_fftSize - 1);

	if(binF < 0.0 || binF >= static_cast<double>(m_fftSize))
		return -DBL_MAX;

	const int b0 = static_cast<int>(binF);
	const int r0 = static_cast<int>(dataRowF);

	if(!m_antialiasing)
		return m_data[r0][b0];

	const int r1 = std::min(r0 + 1, nRows - 1);
	const double ty = dataRowF - r0;

	const int b1 = std::min(b0 + 1, static_cast<int>(m_fftSize) - 1);
	const double tx = binF - b0;

	return (1.0 - ty) * ((1.0 - tx) * m_data[r0][b0] + tx * m_data[r0][b1]) +
		ty * ((1.0 - tx) * m_data[r1][b0] + tx * m_data[r1][b1]);
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

void WaterfallPlotWidget::addFFTData(const float *data, size_t size)
{
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

void WaterfallPlotWidget::setHistorySnapshot(std::vector<QVector<float>> rows)
{
	m_data->setSnapshot(std::move(rows));
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
	xAxis()->setInterval(startHz, stopHz);
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

void WaterfallPlotWidget::setAntialiasing(bool enabled)
{
	m_data->setAntialiasing(enabled);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

void WaterfallPlotWidget::setChannel(ChannelData *ch)
{
	disconnect(m_channel, &ChannelData::newData, this, nullptr);
	if(m_channel != ch)
		clearData();

	m_channel = ch;
	if(ch)
		connect(m_channel, &ChannelData::newData, this,
			[this](const float *, const float *yData, size_t size, bool) {
				if(m_waterfallEnabled) {
					addFFTData(yData, size);
					replot();
				}
			});
}

#include "moc_waterfallplotwidget.cpp"
