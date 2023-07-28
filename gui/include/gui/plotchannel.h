#ifndef PLOTCHANNEL_H
#define PLOTCHANNEL_H

#include <QwtPlotCurve>
#include <QwtPlot>
#include "scopy-gui_export.h"

namespace scopy {
class PlotAxis;
class PlotWidget;
class PlotAxisHandle;
class SCOPY_GUI_EXPORT PlotChannel : public QObject {
	Q_OBJECT
public:
	typedef enum {
		PCS_LINES = 0,
		PCS_DOTS,
		PCS_STEPS,
		PCS_STICKS,
		PCS_SMOOTH
	} PlotCurveStyle;

	PlotChannel(QString name, QPen pen, PlotWidget *plot, PlotAxis* xAxis, PlotAxis *yAxis, QObject *parent = nullptr);
	~PlotChannel();

	QwtPlotCurve *curve() const;
	PlotAxis *xAxis() const;
	PlotAxis *yAxis() const;

	PlotAxisHandle *handle() const;
	void setHandle(PlotAxisHandle *newHandle);

public Q_SLOTS:
	void raise();
	void attach();
	void detach();
	void setEnabled(bool b);
	void enable();
	void disable();

	void setThickness(int);
	void setStyle(int);

private:
	PlotAxis *m_xAxis, *m_yAxis;
	PlotAxisHandle *m_handle;
	QwtPlotCurve *m_curve;
	QwtSymbol *symbol;
	PlotWidget *m_plotWidget;
	QwtPlot *m_plot;
	float *m_data;
};
}

#endif // PLOTCHANNEL_H
