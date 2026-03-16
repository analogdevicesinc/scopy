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

#include "plotbuttonmanager.h"
#include "plotnavigator.hpp"
#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QWidget>
#include <QwtPlot>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_interval.h>
#include <qwt_raster_data.h>
#include <plotinfo.h>

#include <deque>
#include <vector>
#include <cstdint>

namespace scopy {

class PlotAxis;
class PlotNavigator;

/**
 * @brief WaterfallData — simplified QwtRasterData for a rolling FFT waterfall buffer.
 *
 * Stores rows of FFT magnitude data in a deque. The oldest row is at the bottom
 * (Y=0) and the newest at the top (Y=numRows). X maps to frequency bins linearly.
 * Z (intensity) interval controls color map scaling.
 */
class SCOPY_GUI_EXPORT WaterfallData : public QwtRasterData
{
public:
	explicit WaterfallData(int maxRows = 200);
	~WaterfallData() override;

	/**
	 * Push a new row of FFT magnitude data. If the buffer is full the oldest
	 * row is discarded. @p size must be > 0.
	 */
	void addFFTData(const double *data, size_t size);

	/** Clear all stored rows. */
	void reset();

	/** Set the frequency range mapped to the X axis. */
	void setXInterval(double minFreq, double maxFreq);

	/** Set the Y axis range (e.g. 0..numRows or a time range set by the caller). */
	void setYInterval(double min, double max);

	/** Set the intensity (Z) interval used for color map scaling. */
	void setZInterval(double minDb, double maxDb);

	/** Change the maximum number of rows retained. Resets existing data. */
	void setMaxRows(int rows);
	int maxRows() const;

	/** Number of rows currently stored (≤ maxRows). */
	int rowCount() const;

	// QwtRasterData interface
	QwtInterval interval(Qt::Axis axis) const override;
	double value(double x, double y) const override;

private:
	std::deque<std::vector<double>> m_data;
	int m_maxRows;
	size_t m_fftSize; // set on first addFFTData call

	QwtInterval m_xInterval;
	QwtInterval m_yInterval;
	QwtInterval m_zInterval;
};

// ---------------------------------------------------------------------------
// Default color map (black → purple → blue → orange → white)
// ---------------------------------------------------------------------------
class SCOPY_GUI_EXPORT WaterfallColorMap : public QwtLinearColorMap
{
public:
	WaterfallColorMap()
		: QwtLinearColorMap(Qt::black, Qt::white)
	{
		addColorStop(0.16, Qt::black);
		addColorStop(0.33, QColor(58, 36, 59));   // deep purple
		addColorStop(0.50, QColor(74, 100, 255)); // scopy blue
		addColorStop(0.66, QColor(255, 144, 0));  // scopy orange
		addColorStop(0.83, Qt::white);
	}
};

// ---------------------------------------------------------------------------
// WaterfallPlotWidget
// ---------------------------------------------------------------------------
/**
 * @brief WaterfallPlotWidget — a PlotWidget-style QWidget that renders a
 * waterfall (spectrogram) plot using QwtPlotSpectrogram.
 *
 * Interface mirrors PlotWidget as closely as possible so that callers can
 * swap between the two with minimal changes:
 *   - plot()             → underlying QwtPlot*
 *   - xAxis() / yAxis() → PlotAxis objects for frequency and time axes
 *   - navigator()        → PlotNavigator for zoom/pan
 *   - plotButtonManager() → PlotButtonManager overlay
 *   - replot()           → explicit refresh
 *
 * Waterfall-specific API:
 *   - addFFTData(data, size) → push a new spectrum row
 *   - setFrequencyRange(start, stop)
 *   - setYRange(min, max)
 *   - setIntensityRange(minDb, maxDb)
 *   - setNumRows(n)
 *   - setColorMap(map)
 *   - clearData()
 */
class SCOPY_GUI_EXPORT WaterfallPlotWidget : public QWidget
{
	Q_OBJECT
public:
	explicit WaterfallPlotWidget(QWidget *parent = nullptr);
	~WaterfallPlotWidget() override;

	// -----------------------------------------------------------------------
	// Core accessors — same interface as PlotWidget
	// -----------------------------------------------------------------------
	QwtPlot *plot() const;
	QGridLayout *layout();

	PlotAxis *xAxis();
	PlotAxis *yAxis();

	/** Register an externally created PlotAxis with this widget's axis list. */
	void addPlotAxis(PlotAxis *ax);
	QList<PlotAxis *> &plotAxis(int position);
	PlotAxis *plotAxisFromId(QwtAxisId axisId);

	PlotNavigator *navigator() const;
	PlotButtonManager *plotButtonManager() const;
	PlotInfo *getPlotInfo();

	// -----------------------------------------------------------------------
	// Waterfall data API
	// -----------------------------------------------------------------------
	/**
	 * Push one spectrum row. The array must have @p size doubles containing
	 * linear-scale FFT magnitudes (or dBFS — whatever you display).
	 * Thread-safety: call from the GUI thread or post via Qt event.
	 */
	void addFFTData(const double *data, size_t size);

	/** Discard all buffered rows and redraw. */
	void clearData();

	// -----------------------------------------------------------------------
	// Frequency (X) axis
	// -----------------------------------------------------------------------
	void setFrequencyRange(double startHz, double stopHz);
	double startFrequency() const;
	double stopFrequency() const;

	// -----------------------------------------------------------------------
	// Time / row (Y) axis — caller controls the range
	// -----------------------------------------------------------------------
	/**
	 * Set the displayed Y range. The internal WaterfallData always maps
	 * rows 0..numRows; setYRange lets the caller label the axis in real units
	 * (e.g. seconds) by setting the axis min/max on the PlotAxis directly,
	 * or via this convenience method which updates both data and axis.
	 */
	void setYRange(double min, double max);

	// -----------------------------------------------------------------------
	// Intensity / color map (Z axis)
	// -----------------------------------------------------------------------
	void setIntensityRange(double minDb, double maxDb);
	double minIntensity() const;
	double maxIntensity() const;

	/** Scan the current data buffer and auto-scale the intensity range. */

	// -----------------------------------------------------------------------
	// Row count (history depth)
	// -----------------------------------------------------------------------
	void setNumRows(int rows);
	int numRows() const;

	// -----------------------------------------------------------------------
	// Color map
	// -----------------------------------------------------------------------
	/** Replace the current color map. WaterfallPlotWidget takes ownership. */
	void setColorMap(QwtColorMap *colorMap);

	// -----------------------------------------------------------------------
	// Color bar (right-axis legend)
	// -----------------------------------------------------------------------
	void setColorBarVisible(bool visible);
	void setColorBarTitle(const QString &title);

	// -----------------------------------------------------------------------
	// Axis label visibility — same API as PlotWidget
	// -----------------------------------------------------------------------
	bool showXAxisLabels() const;
	void setShowXAxisLabels(bool visible);
	bool showYAxisLabels() const;
	void setShowYAxisLabels(bool visible);
	void setUnitsVisible(bool visible);

	// -----------------------------------------------------------------------
	// Misc
	// -----------------------------------------------------------------------
	void setAlignCanvasToScales(bool align);
	bool eventFilter(QObject *object, QEvent *event) override;

public Q_SLOTS:
	void replot();
	void showAxisLabels();
	void hideAxisLabels();
	void autoScaleIntensity();

Q_SIGNALS:
	void canvasSizeChanged();
	void mouseButtonPress(const QMouseEvent *event);
	void mouseButtonRelease(const QMouseEvent *event);
	void mouseMove(const QMouseEvent *event);
	void plotScaleChanged();
	void newData();

protected:
	QSize minimumSizeHint() const override;

private:
	// Core Qt/Qwt objects
	QwtPlot *m_plot;
	QGridLayout *m_layout;

	// Spectrogram items
	QwtPlotSpectrogram *m_spectrogram;
	WaterfallData *m_data;

	// Axis infrastructure (mirrors PlotWidget)
	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;
	QList<PlotAxis *> m_plotAxis[QwtAxis::AxisPositions];

	// Navigation / overlays
	PlotNavigator *m_navigator;
	PlotInfo *m_plotInfo;
	PlotButtonManager *m_plotButtonManager;

	// State
	bool m_showXAxisLabels;
	bool m_showYAxisLabels;
	int m_xPosition;
	int m_yPosition;

	double m_startFreq;
	double m_stopFreq;

	// Setup helpers
	void setupOpenGLCanvas();
	void setupAxes();
	void setupNavigator();
	void setupPlotInfo();
	void setupPlotButtonManager();
	void setupColorBar();
	void updateColorBar();
};

} // namespace scopy

#endif // WATERFALL_PLOT_WIDGET_H
