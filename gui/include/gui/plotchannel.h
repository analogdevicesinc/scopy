#ifndef PLOTCHANNEL_H
#define PLOTCHANNEL_H

#include "scopy-gui_export.h"

#include <QPen>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotMarker>
#include <QwtSymbol>

namespace scopy {
class PlotAxis;
class PlotWidget;
class PlotAxisHandle;
class SCOPY_GUI_EXPORT PlotChannel : public QObject
{
	Q_OBJECT
public:
	typedef enum
	{
		PCS_LINES = 0,
		PCS_DOTS,
		PCS_STEPS,
		PCS_STICKS,
		PCS_SMOOTH
	} PlotCurveStyle;

	PlotChannel(QString name, QPen pen, PlotAxis *xAxis, PlotAxis *yAxis, QObject *parent = nullptr);
	~PlotChannel();

	QwtPlotCurve *curve() const;
	PlotAxis *xAxis() const;
	PlotAxis *yAxis() const;

	PlotAxisHandle *handle() const;
	void setHandle(PlotAxisHandle *newHandle);

	QList<QwtPlotMarker *> markers();
	QwtPlotMarker *buildMarker(QString str, QwtSymbol::Style shape, double x, double y);
	void clearMarkers();
	void removeMarker(QwtPlotMarker *m);
	void addMarker(QwtPlotMarker *m);
	void setSamples(const float* xData, const float* yData, size_t size, bool copy = true);

	QString name() const;

public Q_SLOTS:
	void raise();
	void attach();
	void detach();
	void setEnabled(bool b);
	void enable();
	void disable();

	void setThickness(int);
	void setStyle(int);

Q_SIGNALS:
	void attachCurve(QwtPlotCurve *curve);
	void doReplot();
	void newData(const float *xData, const float *yData, size_t size, bool);

private:
	PlotAxis *m_xAxis, *m_yAxis;
	PlotAxisHandle *m_handle;
	QwtPlotCurve *m_curve;
	QList<QwtPlotMarker *> m_markers;
	QwtSymbol *symbol;
	QPen m_pen;
	float *m_data;
	QString m_name;
};
} // namespace scopy

#endif // PLOTCHANNEL_H
