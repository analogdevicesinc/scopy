#ifndef MARKERCONTROLLER_H
#define MARKERCONTROLLER_H

#include "plot_utils.hpp"
#include "plotaxishandle.h"
#include "qboxlayout.h"
#include "qtextedit.h"
#include "qwidget.h"
#include "utils.h"
#include <QObject>
#include <QwtPlotMarker>
#include <QwtSymbol>
#include <scopy-adc_export.h>

namespace scopy {
namespace adc {

class FFTPlotComponentChannel;
class SCOPY_ADC_EXPORT MarkerController : public QObject
{
	Q_OBJECT
public:
	typedef enum {
		MC_NONE,
		MC_PEAK,
		MC_FIXED,
		MC_SINGLETONE,
		MC_IMAGE
	} MarkerTypes;

	typedef struct {
		double x;
		double y;
		int idx;
	} PeakInfo;

	typedef struct {
		QString name;
		QwtPlotMarker *marker;
		PeakInfo peak;
	} MarkerInfo;

	MarkerController(FFTPlotComponentChannel *ch, QObject *parent);
	~MarkerController();


	void init();
	bool enabled() const;
	void setEnabled(bool newEnabled);
	void attachMarkersToPlot();
	const QList<MarkerInfo> &markerInfo() const;

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

private:
	void initFixedMarker();
	void deinitFixedMarker();
	void computePeaks();
	void computePeakMarkers();
	void computeSingleToneMarkers();
	void computeFixedMarkerFrequency();
	void computeImageMarkers();	
	int findPeakNearIdx(int idx, int range);

	void cacheMarkerInfo();
	double popCacheMarkerInfo();

	bool m_enabled;
	int m_nrOfMarkers;

	bool m_complex;
	bool m_handlesVisible;

	QList<QwtPlotMarker*> m_markers;
	QList<PeakInfo> m_peakInfo;
	QList<PeakInfo> m_sortedPeakInfo;
	QList<MarkerInfo> m_markerInfo;
	QList<PlotAxisHandle*> m_fixedHandles;
	QList<double> m_markerCache;

	MarkerTypes m_markerType;

	FFTPlotComponentChannel *m_ch;

	QwtPlot *m_plot;
	QwtAxisId m_xAxis;
	QwtAxisId m_yAxis;


};

class SCOPY_ADC_EXPORT MarkerPanel : public QWidget
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
	QMap<QString, QWidget*> m_map;
};

class SCOPY_ADC_EXPORT MarkerLabel : public QWidget
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
}
}

#endif // MARKERCONTROLLER_H
