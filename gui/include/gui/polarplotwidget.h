#ifndef POLARPLOTWIDGET_H
#define POLARPLOTWIDGET_H

#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QWidget>
#include <polarplotchannel.h>
#include <qwt_plot_zoomer.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_plot.h>

namespace scopy {
class SCOPY_GUI_EXPORT PolarPlotWidget : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		MGS_LINES = 0,
		MGS_DASH,
		MGS_DOT,
		MGS_DASH_DOT,
		MGS_DASH_DOT_DOT
	} MinorGridStyle;

	PolarPlotWidget(QWidget *parent = nullptr);
	~PolarPlotWidget();

	void addPlotChannel(PolarPlotChannel *ch);
	void removePlotChannel(PolarPlotChannel *ch);

	QwtPolarPlot *plot() const;
	void setAzimuthInterval(double min, double max, double step = 0.0);
	void setRadiusInterval(double min, double max, double step = 0.0);

	void showRadiusAxis(bool left = true, bool right = true, bool top = true, bool bottom = true);
	void showAzimuthAxis(bool show = true);

	void setGridPen(QPen pen);
	void showMinorGrid(bool show = true);
	void setMinorGridPen(int style, QPen pen = QPen(Qt::gray));

	void setBgColor(const QColor &color);

	void setData(QVector<QVector<QwtPointPolar>> data);

public Q_SLOTS:
	void replot();

private:
	void setupZoomer();

	QwtPolarPlot *m_plot;
	QwtPolarGrid *m_grid;
	QList<PolarPlotChannel *> m_plotChannels;
	QGridLayout *m_layout;
};
} // namespace scopy

#endif // POLARPLOTWIDGET_H
