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
#include <algorithm>
#include <cfloat>
#include <cstring>
#include <vector>

using namespace scopy;

// =============================================================================
// WaterfallData
// =============================================================================

WaterfallData::WaterfallData()
	: QwtRasterData()
	, m_head(0)
	, m_count(0)
	, m_maxRows(0)
	, m_fftSize(0)
	, m_antialiasing(true)
	, m_xInterval(0.0, 1.0)
	, m_zInterval(-120.0, 0.0)
{}

WaterfallData::~WaterfallData() {}

void WaterfallData::rebuildRing()
{
	if(m_maxRows <= 0 || m_fftSize == 0) {
		m_ring.clear();
		m_head = 0;
		m_count = 0;
		return;
	}

	// The slot count the existing block was laid out with, which is not m_maxRows —
	// this runs *after* m_maxRows changed, and row() would otherwise wrap the old
	// data modulo the new size.
	const int oldRows = m_fftSize > 0 ? static_cast<int>(m_ring.size() / m_fftSize) : 0;
	const int keep = std::min(m_count, m_maxRows);

	std::vector<float> next(static_cast<size_t>(m_maxRows) * m_fftSize, -FLT_MAX);
	// The newest `keep` rows, oldest-first from slot 0, so the result starts unwrapped
	// and the copy is a single pass.
	for(int i = 0; oldRows > 0 && i < keep; ++i) {
		int slot = (m_head - keep + i) % oldRows;
		if(slot < 0)
			slot += oldRows;
		std::memcpy(next.data() + static_cast<size_t>(i) * m_fftSize,
			    m_ring.data() + static_cast<size_t>(slot) * m_fftSize, m_fftSize * sizeof(float));
	}
	m_ring = std::move(next);
	m_count = oldRows > 0 ? keep : 0;
	m_head = m_count % m_maxRows;
}

void WaterfallData::pushRow(const float *data, size_t len)
{
	float *dst = m_ring.data() + static_cast<size_t>(m_head) * m_fftSize;
	std::memcpy(dst, data, len * sizeof(float));
	if(len < m_fftSize)
		std::fill(dst + len, dst + m_fftSize, -FLT_MAX);
	m_head = (m_head + 1) % m_maxRows;
	if(m_count < m_maxRows)
		++m_count;
}

void WaterfallData::appendRow(const float *data, size_t size)
{
	// m_maxRows == 0 means setMaxRows() has not run yet. Dropping the row is the old
	// behaviour; the difference is that it no longer latches m_fftSize on the way out,
	// which used to leave value() reading a ring that was never filled.
	if(!data || size == 0 || m_maxRows <= 0)
		return;

	if(size != m_fftSize) {
		// A bin-count change invalidates every stored row — they are spectra of a
		// different width. Same semantics as the old addFFTData, which cleared.
		m_fftSize = size;
		m_ring.assign(static_cast<size_t>(m_maxRows) * m_fftSize, -FLT_MAX);
		m_head = 0;
		m_count = 0;
	} else if(m_ring.size() != static_cast<size_t>(m_maxRows) * m_fftSize) {
		// setMaxRows() ran before the geometry was known, or the allocation is
		// otherwise stale. Sized here so a row count set ahead of the first frame
		// costs nothing.
		rebuildRing();
		if(m_ring.empty())
			return;
	}

	pushRow(data, size);
}

void WaterfallData::addFFTData(const float *data, size_t size) { appendRow(data, size); }

void WaterfallData::setSnapshot(std::vector<QVector<float>> rows)
{
	// rows[0]=newest, rows.back()=oldest (SampleBuffer convention). The ring stores
	// oldest-first, so it is filled back-to-front.
	if(rows.empty() || m_maxRows <= 0) {
		reset();
		return;
	}
	const size_t bins = static_cast<size_t>(rows.front().size());
	if(bins == 0) {
		reset();
		return;
	}

	// Resize once here rather than letting the first appendRow() do it, so a snapshot
	// of the same geometry as the previous one reuses the existing allocation.
	m_fftSize = bins;
	if(m_ring.size() != static_cast<size_t>(m_maxRows) * m_fftSize)
		m_ring.assign(static_cast<size_t>(m_maxRows) * m_fftSize, -FLT_MAX);
	m_head = 0;
	m_count = 0;

	// Only the newest m_maxRows are drawable, so the rest are not copied at all —
	// the old code built a vector for every row and then popped the excess.
	const int n = std::min(static_cast<int>(rows.size()), m_maxRows);
	for(int i = n - 1; i >= 0; --i) {
		const QVector<float> &r = rows[static_cast<size_t>(i)];
		// pushRow, not appendRow: a row shorter than rows[0] is padded, not taken
		// for a width change. The old code latched m_fftSize from rows[0] and let
		// value() index every row to it, reading past the end of any shorter one.
		if(!r.isEmpty())
			pushRow(r.constData(), std::min(static_cast<size_t>(r.size()), m_fftSize));
	}
}

void WaterfallData::reset()
{
	// m_ring and m_fftSize are kept: a stop/start keeps the same geometry, so the
	// next append reuses the block instead of reallocating it.
	m_head = 0;
	m_count = 0;
}

void WaterfallData::setXInterval(double minFreq, double maxFreq) { m_xInterval = QwtInterval(minFreq, maxFreq); }

void WaterfallData::setZInterval(double minDb, double maxDb) { m_zInterval = QwtInterval(minDb, maxDb); }

void WaterfallData::setMaxRows(int rows)
{
	if(rows <= 0 || rows == m_maxRows)
		return;
	m_maxRows = rows;
	// Resized here, keeping the newest rows. The old version left m_data untouched,
	// so shrinking kept the excess rows live until the next write.
	if(m_fftSize > 0)
		rebuildRing();
}

int WaterfallData::maxRows() const { return m_maxRows; }

int WaterfallData::rowCount() const { return m_count; }

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

	if(m_count == 0 || m_fftSize == 0)
		return -DBL_MAX;

	const int nRows = m_count;

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

	// Resolved once per pixel instead of once per tap: four deque double-indirections
	// become two index computations into one contiguous block.
	const float *p0 = row(r0);
	if(!p0)
		return -DBL_MAX;

	if(!m_antialiasing)
		return p0[b0];

	const int r1 = std::min(r0 + 1, nRows - 1);
	const double ty = dataRowF - r0;

	const int b1 = std::min(b0 + 1, static_cast<int>(m_fftSize) - 1);
	const double tx = binF - b0;

	const float *p1 = (r1 == r0) ? p0 : row(r1);

	return (1.0 - ty) * ((1.0 - tx) * p0[b0] + tx * p0[b1]) + ty * ((1.0 - tx) * p1[b0] + tx * p1[b1]);
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
	appendRowDeferred(data, size);
	endAppend();
}

void WaterfallPlotWidget::appendRowDeferred(const float *data, size_t size)
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

	m_data->appendRow(data, size);
}

void WaterfallPlotWidget::endAppend()
{
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
	// The row count feeds interval(Qt::YAxis) and value()'s row mapping, so a stale
	// raster cache here draws the old geometry until the next append happens to
	// invalidate it.
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

void WaterfallPlotWidget::setAntialiasing(bool enabled)
{
	m_data->setAntialiasing(enabled);
	m_spectrogram->invalidateCache();
	m_spectrogram->itemChanged();
}

void WaterfallPlotWidget::setChannel(ChannelData *ch)
{
	if(m_channel)
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
