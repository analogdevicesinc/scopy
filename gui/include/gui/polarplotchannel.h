#ifndef POLARPLOTCHANNEL_H
#define POLARPLOTCHANNEL_H

#include "scopy-gui_export.h"
#include <QObject>
#include <qwt_polar_curve.h>
#include <qwt_polar_marker.h>



namespace scopy {
class PolarPlotWidget;
class SCOPY_GUI_EXPORT PolarPlotChannel : public QObject
{
	Q_OBJECT
public:
	PolarPlotChannel(QString name, QPen pen, PolarPlotWidget *plot, QObject *parent = nullptr);
	~PolarPlotChannel();

	QwtPolarCurve *curve() const;
	void setSamples(QVector<QwtPointPolar> samples);

public Q_SLOTS:
	void raise();
	void attach();
	void detach();
	void setEnabled(bool b);
	void enable();
	void disable();

	void setThickness(int);

private:
	QwtPolarCurve *m_curve;
	QwtSymbol *m_symbol;
	QwtPolarPlot *m_plot;
	QPen m_pen;
	QwtArraySeriesData<QwtPointPolar> *m_data;
};
}

#endif // POLARPLOTCHANNEL_H
