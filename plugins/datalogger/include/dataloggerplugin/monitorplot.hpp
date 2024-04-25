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
#include "scopy-dataloggerplugin_export.h"

namespace scopy {

class TimePlotInfo;
namespace datamonitor {

class MonitorPlotCurve;

class SCOPY_DATALOGGERPLUGIN_EXPORT MonitorPlot : public QWidget
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
