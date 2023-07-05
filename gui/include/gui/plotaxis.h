#ifndef PLOTAXIS_H
#define PLOTAXIS_H

#include "scopy-gui_export.h"
#include <QwtScaleEngine>
#include "plotwidget.h"
#include <DisplayPlot.h>
#include "osc_scale_engine.h"
#include "plotaxishandle.h"
#include <QwtPlotZoomer>

namespace scopy {
class SCOPY_GUI_EXPORT PlotAxis : public QObject {
	Q_OBJECT
public:
	PlotAxis(int position, PlotWidget *p, QObject *parent = nullptr);
	~PlotAxis() {}

	int position();
	bool isHorizontal();
	bool isVertical();
	QwtPlot* plot();
	void setDivs(double);
	void setInterval(double min, double max);

	const QwtAxisId &axisId() const;

	double min() const;
	double max() const;

	QwtPlotZoomer *zoomer() const;

public Q_SLOTS:
	void setVisible(bool);
	void updateAxisScale();

private:
	QwtAxisId m_axisId;
	PlotWidget *m_plotWidget;
	QwtPlotZoomer *m_zoomer;
	QwtPlot *m_plot;
	int m_position;
	OscScaleDraw *m_scaleDraw;
	OscScaleEngine *m_scaleEngine;

	PlotAxisHandle *offsetHandle;
	int m_id;
	double m_divs;
	double m_min;
	double m_max;

	void setupAxisScale();
	void setupZoomer();
};
}
#endif // PLOTAXIS_H
