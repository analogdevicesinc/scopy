#ifndef MONITORPLOT_H
#define MONITORPLOT_H

#include <QWidget>
#include <QMap>
#include <plotwidget.h>
#include <plotaxis.h>
#include <datamonitormodel.hpp>
#include <QwtDateScaleDraw>
#include <QwtDateScaleEngine>
#include <QTimer>
#include <plotbufferpreviewer.h>
#include "scopy-datamonitorplugin_export.h"

namespace scopy {

namespace datamonitor {

class MonitorPlotCurve;

class SCOPY_DATAMONITORPLUGIN_EXPORT MonitorPlot : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit MonitorPlot(QWidget *parent = nullptr);
	~MonitorPlot();

	PlotWidget *plot() const;
	void addMonitor(DataMonitorModel *dataMonitorModel);
	void removeMonitor(QString monitorName);
	void toggleMonitor(bool toggled, QString monitorName);
	bool hasMonitor(QString title);

	void changeCurveStyle(int style);
	void changeCurveThickness(double thickness);

	void updateXAxisIntervalMin(double min);
	void updateXAxisIntervalMax(double max);
	void updateYAxisIntervalMin(double min);
	void updateYAxisIntervalMax(double max);
	void plotYAxisAutoscale();
	double getYAxisIntervalMin();
	double getYAxisIntervalMax();

	void setIsRealTime(bool newIsRealTime);

	void setStartTime();

	void updatePlotStartingPoint(double time, double delta);

private:
	PlotWidget *m_plot;
	QMap<QString, MonitorPlotCurve *> *m_monitorCurves = nullptr;
	bool m_firstMonitor = true;

	int m_currentCurveStyle = 0;
	double m_currentCurveThickness = 1;
	bool m_isRealTime = false;

	void setupXAxis();
	void genereateScaleDraw(QString format, double offset);
	QwtDateScaleEngine *scaleEngine = nullptr;

	double m_startTime = 0;

	void updateAxisScaleDraw();
	void refreshXAxisInterval();
	double m_xAxisIntervalMin;
	double m_xAxisIntervalMax;

	PlotBufferPreviewer *m_bufferPreviewer;
};
} // namespace datamonitor
} // namespace scopy
#endif // MONITORPLOT_H
