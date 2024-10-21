#ifndef MARKERCONTROLLER_H
#define MARKERCONTROLLER_H

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
class SCOPY_GUI_EXPORT MarkerController : public QObject
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

	MarkerController(PlotComponentChannel *ch, QObject *parent);
	MarkerController(QObject *parent);
	~MarkerController();

	void init();
	bool enabled() const;
	void setEnabled(bool newEnabled);
	virtual void attachMarkersToPlot() = 0;
	const QList<MarkerInfo> &markerInfo() const;

	PlotComponentChannel *ch() const;
	void setCh(PlotComponentChannel *newCh);

public Q_SLOTS:
	void setNrOfMarkers(int);
	void setMarkerType(MarkerTypes);
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
	void updateChannel(QString, QList<MarkerController::MarkerInfo>);
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
	void updateInfo(QList<MarkerController::MarkerInfo> m);

private:
	QVBoxLayout *m_lay;
	QString m_name;
	QTextEdit *m_txt;
	MetricPrefixFormatter *m_mpf;
};
} // namespace scopy

#endif // MARKERCONTROLLER_H
