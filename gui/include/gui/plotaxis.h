#ifndef PLOTAXIS_H
#define PLOTAXIS_H

#include "osc_scale_engine.h"
#include "plotaxishandle.h"
#include "plotwidget.h"
#include "scopy-gui_export.h"

#include <QwtPlotZoomer>
#include <QwtScaleEngine>

#include <DisplayPlot.h>

namespace scopy {
class SCOPY_GUI_EXPORT PlotAxis : public QObject
{
	Q_OBJECT
public:
	PlotAxis(int position, PlotWidget *p, QPen pen, QObject *parent = nullptr);
	~PlotAxis() {}

	int position();
	bool isHorizontal();
	bool isVertical();
	QwtPlot *plot();
	void setDivs(double);
	void setInterval(double min, double max);

	const QwtAxisId &axisId() const;

	double min() const;
	double max() const;
	double visibleMin() const;
	double visibleMax() const;

	QwtPlotZoomer *zoomer() const;
	OscScaleEngine *scaleEngine() const;

	OscScaleDraw *scaleDraw() const;

	double divs() const;

	void setFromatter(PrefixFormatter *formatter);
	PrefixFormatter *getFromatter();

	void setUnits(QString units);
	QString getUnits();

	void setUnitsVisible(bool visible);

public Q_SLOTS:
	void setMax(double newMax);
	void setMin(double newMin);

	void setVisible(bool);
	void updateAxisScale();

Q_SIGNALS:
	void axisScaleUpdated();
	void maxChanged(double);
	void minChanged(double);
	void formatterChanged(PrefixFormatter *formatter);
	void unitsChanged(QString units);

private:
	QwtAxisId m_axisId;
	PlotWidget *m_plotWidget;
	QwtPlot *m_plot;
	int m_position;
	OscScaleDraw *m_scaleDraw;
	OscScaleEngine *m_scaleEngine;
	QString m_units;
	PrefixFormatter *m_formatter;

	int m_id;
	double m_divs;
	double m_min;
	double m_max;

	void setupAxisScale();
	Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)
	Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
};
} // namespace scopy
#endif // PLOTAXIS_H
