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

#include <deque>
#include <vector>

namespace scopy {

class SCOPY_GUI_EXPORT WaterfallData : public QwtRasterData
{
public:
	explicit WaterfallData();
	~WaterfallData() override;

	void addFFTData(const float *data, size_t size);
	void reset();

	void setXInterval(double minFreq, double maxFreq);
	void setZInterval(double minDb, double maxDb);

	void setMaxRows(int rows);
	int maxRows() const;
	int rowCount() const;

	void setInverted(bool inverted);
	bool inverted() const;

	QwtInterval interval(Qt::Axis axis) const override;
	double value(double x, double y) const override;

private:
	std::deque<std::vector<float>> m_data;
	int m_maxRows;
	size_t m_fftSize;
	bool m_inverted;

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
	void clearData();

	void setChannel(ChannelData *ch);

	void setFrequencyRange(double startHz, double stopHz);
	void setIntensityRange(double minDb, double maxDb);
	void setNumRows(int rows);
	void setInverted(bool inverted);

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
