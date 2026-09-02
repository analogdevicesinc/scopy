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

#ifndef WATERFALL_PLOT_WIDGET_H
#define WATERFALL_PLOT_WIDGET_H

#include "plotwidget.h"
#include "toolcomponent.h"
#include "scopy-gui_export.h"

#include <qwt_plot_spectrogram.h>
#include <qwt_interval.h>
#include <qwt_raster_data.h>
#include <QwtLinearColorMap>
#include <QElapsedTimer>
#include <plot_utils.hpp>

#include <vector>

namespace scopy {

class SCOPY_GUI_EXPORT WaterfallData : public QwtRasterData
{
public:
	explicit WaterfallData();
	~WaterfallData() override;

	// Append one row, becoming the newest. Nothing is retained by reference: the
	// samples are memcpy'd straight into the ring slot, so the caller may hand over
	// a view into a chunk or a scratch buffer and reuse it immediately afterwards.
	void appendRow(const float *data, size_t size);
	void addFFTData(const float *data, size_t size);
	// Replace the entire history with an externally-managed snapshot.
	// rows[0] = newest frame, rows.back() = oldest frame (SampleBuffer convention).
	//
	// A bulk fill, not the incremental path: every row is copied. Callers with a
	// growing history want appendRow() instead — see AcqWaterfallChannel.
	void setSnapshot(std::vector<QVector<float>> rows);
	void reset();

	void setXInterval(double minFreq, double maxFreq);
	void setZInterval(double minDb, double maxDb);

	void setMaxRows(int rows);
	int maxRows() const;
	int rowCount() const;

	void setAntialiasing(bool enabled);

	QwtInterval interval(Qt::Axis axis) const override;
	double value(double x, double y) const override;

private:
	// Reallocate m_ring for the current m_maxRows x m_fftSize, preserving the newest
	// min(m_count, m_maxRows) rows. The only allocating path besides a bin-count
	// change, and it runs on a settings change rather than per frame.
	void rebuildRing();

	// Copy one row into the next ring slot and advance. Assumes m_ring is already
	// sized for the current geometry. `len` shorter than m_fftSize is padded rather
	// than treated as a width change, which is what lets setSnapshot() accept a
	// ragged snapshot without each short row wiping the rows before it.
	void pushRow(const float *data, size_t len);

	// Logical row index -> the row's samples. r = 0 is the oldest live row,
	// r = m_count - 1 the newest. Null when the ring is unallocated, so value() can
	// bail rather than dereference.
	const float *row(int r) const
	{
		if(m_ring.empty())
			return nullptr;
		int slot = (m_head - m_count + r) % m_maxRows;
		if(slot < 0)
			slot += m_maxRows;
		return m_ring.data() + static_cast<size_t>(slot) * m_fftSize;
	}

	// One contiguous block of m_maxRows * m_fftSize floats, used as a ring of rows.
	// m_head is the slot the next row goes into; m_count is how many are live.
	//
	// Contiguous rather than a deque of per-row vectors because both hot paths care:
	// appending becomes a memcpy into an existing slot with no allocation, and
	// value() — called once per raster pixel per repaint — becomes index arithmetic
	// over one cache-friendly block instead of a double indirection per tap.
	std::vector<float> m_ring;
	int m_head;
	int m_count;

	int m_maxRows;
	size_t m_fftSize;
	bool m_antialiasing;

	QwtInterval m_xInterval;
	QwtInterval m_zInterval;
};

class SCOPY_GUI_EXPORT WaterfallColorMap : public QwtLinearColorMap
{
public:
	WaterfallColorMap()
		: QwtLinearColorMap(Qt::black, Qt::white)
	{
		addColorStop(0.16, Qt::black);
		addColorStop(0.33, QColor(58, 36, 59));	  // deep purple
		addColorStop(0.50, QColor(74, 100, 255)); // scopy blue
		addColorStop(0.66, QColor(255, 144, 0));  // scopy orange
		addColorStop(0.83, Qt::white);
	}
};

// Formatter that converts row indices to seconds for display on the Y axis.
class SCOPY_GUI_EXPORT WaterfallTimeFormatter : public MetricPrefixFormatter
{
public:
	explicit WaterfallTimeFormatter(QObject *parent = nullptr);
	void setSecsPerRow(double secs);
	QString format(double value, QString unitType, int precision) const override;

private:
	double m_secsPerRow;
};

class SCOPY_GUI_EXPORT WaterfallPlotWidget : public PlotWidget
{
	Q_OBJECT
public:
	explicit WaterfallPlotWidget(QWidget *parent = nullptr);
	~WaterfallPlotWidget() override;

	void addFFTData(const float *data, size_t size);

	// The same append, split so a caller with several rows to add pays for one
	// repaint instead of one per row. appendRowDeferred() still maintains the Y
	// axis's row timing; only the cache invalidation is held back until endAppend().
	// Every appendRowDeferred() run must be closed by exactly one endAppend().
	void appendRowDeferred(const float *data, size_t size);
	void endAppend();

	// Replace the waterfall history from an external snapshot (e.g. DataStore history).
	// rows[0] = newest frame, rows.back() = oldest (matches SampleBuffer::sample() order).
	void setHistorySnapshot(std::vector<QVector<float>> rows);
	void clearData();

	void setChannel(ChannelData *ch);

	void setFrequencyRange(double startHz, double stopHz);
	void setIntensityRange(double minDb, double maxDb);
	void setNumRows(int rows);
	void setAntialiasing(bool enabled);

public Q_SLOTS:
	void updateYAxis();
	void setWaterfallEnabled(bool enabled);

private:
	QwtPlotSpectrogram *m_spectrogram;
	WaterfallData *m_data;
	WaterfallTimeFormatter *m_timeFormatter;

	QElapsedTimer m_rowTimer;
	double m_secsPerRow;
	int m_rowCount;
	bool m_waterfallEnabled;
	ChannelData *m_channel = nullptr;
};

} // namespace scopy

#endif // WATERFALL_PLOT_WIDGET_H
