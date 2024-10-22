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

#ifndef MONITORPLOT_H
#define MONITORPLOT_H

#include <QWidget>
#include <QMap>
#include <plotwidget.h>
#include <plotaxis.h>
#include <datamonitor/datamonitormodel.hpp>
#include <QwtDateScaleDraw>
#include <QwtDateScaleEngine>
#include <QTimer>
#include <QLabel>
#include <plotbufferpreviewer.h>
#include "scopy-datalogger_export.h"

namespace scopy {

class TimePlotInfo;
namespace datamonitor {

class MonitorPlotCurve;

class SCOPY_DATALOGGER_EXPORT MonitorPlot : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit MonitorPlot(QWidget *parent = nullptr);

	PlotWidget *plot() const;
	void addMonitor(DataMonitorModel *dataMonitorModel);
	void removeMonitor(QString monitorName);
	void toggleMonitor(bool toggled, QString monitorName);
	bool hasMonitor(QString title);

	void updateXAxisIntervalMin(double min);
	void updateXAxisIntervalMax(double max);
	void updateYAxisIntervalMin(double min);
	void updateYAxisIntervalMax(double max);
	double getYAxisIntervalMin();
	double getYAxisIntervalMax();

	void setIsRealTime(bool newIsRealTime);

	void setStartTime();

	void updatePlotStartingPoint(double time, double delta);

	void toggleBufferPreview(bool toggled);
	void updateBufferPreviewer(double time);

Q_SIGNALS:
	void monitorCurveAdded(PlotChannel *c);
	void monitorCurveRemoved(PlotChannel *c);

private:
	QVBoxLayout *layout;
	QString dateTimeFormat;
	PlotWidget *m_plot;
	PlotBufferPreviewer *m_bufferPreviewer;
	QwtDateScaleDraw *m_scaleDraw;
	QMap<QString, MonitorPlotCurve *> *m_monitorCurves;
	bool m_firstMonitor = true;

	int m_currentCurveStyle = 0;
	double m_currentCurveThickness = 1;
	bool m_isRealTime = false;

	void generateBufferPreviewer();
	void setupXAxis();
	void genereateScaleDraw(QString format, double offset);
	QwtDateScaleEngine *scaleEngine;

	double m_startTime = 0;

	void updateAxisScaleDraw();
	void refreshXAxisInterval();
	double m_xAxisIntervalMin;
	double m_xAxisIntervalMax;

	QLabel *startTimeLabel;
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORPLOT_H
