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

#ifndef PLOTMARKERCONTROLLER_H
#define PLOTMARKERCONTROLLER_H

#include <scopy-gui_export.h>
#include "plot_utils.hpp"
#include "plotaxishandle.h"
#include "qboxlayout.h"
#include "qtextedit.h"
#include "qwidget.h"
#include "utils.h"
#include <QObject>
#include <QwtPlotMarker>
#include <QwtSymbol>

namespace scopy {

class PlotComponentChannel;
class SCOPY_GUI_EXPORT PlotMarkerController : public QObject
{
	Q_OBJECT
public:
	typedef enum
	{
		MC_NONE,
		MC_PEAK,
		MC_FIXED,
		MC_SINGLETONE,
		MC_IMAGE
	} MarkerTypes;

	typedef struct
	{
		double x;
		double y;
		int idx;
	} PeakInfo;

	typedef struct
	{
		QString name;
		QwtPlotMarker *marker;
		PeakInfo peak;
	} MarkerInfo;

	PlotMarkerController(PlotComponentChannel *ch, QObject *parent);
	PlotMarkerController(QObject *parent);
	~PlotMarkerController();

	void init();
	bool enabled() const;
	void setEnabled(bool newEnabled);
	virtual void attachMarkersToPlot() = 0;
	const QList<MarkerInfo> &markerInfo() const;

	PlotComponentChannel *ch() const;
	void setCh(PlotComponentChannel *newCh);

public Q_SLOTS:
	void setNrOfMarkers(int);
	void setMarkerType(PlotMarkerController::MarkerTypes);
	void computeMarkers();
	void setAxes(QwtAxisId x, QwtAxisId y);
	void setPlot(QwtPlot *);
	void setComplex(bool b);
	void setFixedHandleVisible(bool b);
	void setFixedMarkerFrequency(int idx, double freq);

Q_SIGNALS:
	void markerInfoUpdated();
	void markerEnabled(bool b);

protected:
	virtual int findPeakNearIdx(int idx, int range) = 0;
	virtual void computeImageMarkers() = 0;
	virtual void computeSingleToneMarkers() = 0;
	virtual void computePeaks() = 0;
	virtual void initFixedMarker() = 0;

	QwtPlot *m_plot;
	QwtAxisId m_xAxis;
	QwtAxisId m_yAxis;
	bool m_complex;
	int m_nrOfMarkers;

	QList<QwtPlotMarker *> m_markers;
	QList<MarkerInfo> m_markerInfo;
	QList<PeakInfo> m_peakInfo;
	QList<PeakInfo> m_sortedPeakInfo;
	QList<PlotAxisHandle *> m_fixedHandles;

	void cacheMarkerInfo();
	double popCacheMarkerInfo();
	bool m_handlesVisible;

private:
	void deinitFixedMarker();
	void computePeakMarkers();
	void computeFixedMarkerFrequency();

	bool m_enabled;

	QList<double> m_markerCache;

	MarkerTypes m_markerType;

	PlotComponentChannel *m_ch;
};

class SCOPY_GUI_EXPORT MarkerPanel : public QWidget
{
	Q_OBJECT;
	QWIDGET_PAINT_EVENT_HELPER;

public:
	MarkerPanel(QWidget *parent = nullptr);
	~MarkerPanel();
public Q_SLOTS:
	void newChannel(QString name, QPen c);
	void deleteChannel(QString name);
	void updateChannel(QString, QList<PlotMarkerController::MarkerInfo>);
	int markerCount();

private:
	QHBoxLayout *m_panelLayout;
	QMap<QString, QWidget *> m_map;
};

class SCOPY_GUI_EXPORT MarkerLabel : public QWidget
{
	Q_OBJECT;
	QWIDGET_PAINT_EVENT_HELPER;

public:
	MarkerLabel(QString name, QPen c, QWidget *parent = nullptr);
	~MarkerLabel();
	QString name();
	void updateInfo(QList<PlotMarkerController::MarkerInfo> m);

private:
	QVBoxLayout *m_lay;
	QString m_name;
	QTextEdit *m_txt;
	MetricPrefixFormatter *m_mpf;
};
} // namespace scopy

#endif // PLOTMARKERCONTROLLER_H
