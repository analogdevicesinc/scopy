#ifndef PLOTGRID_H
#define PLOTGRID_H

#include "scopy-gui_export.h"

#include <QObject>
#include <QwtPlot>
#include <plotwidget.h>

class QwtPlotGrid;

class QwtPlotScaleItem;
namespace scopy {

class SCOPY_GUI_EXPORT PlotScales : public QObject
{
	Q_OBJECT
public:
	PlotScales(PlotWidget *plot);
	~PlotScales();

	void setGridEn(bool en);
	bool getGridEn();

	void setGraticuleEn(bool en);
	bool getGraticuleEn();

protected:
	void initGrid();
	void initGraticule();
	void initMarginScales();

	void setMarginScalesEn(bool en);
	bool getMarginScalesEn();

private:
	PlotWidget *m_plot;
	QwtPlotGrid *m_grid;
	QwtPlotScaleItem *m_y1Graticule;
	QwtPlotScaleItem *m_y2Graticule;
	QwtPlotScaleItem *m_x1Graticule;
	QwtPlotScaleItem *m_x2Graticule;
	bool m_gridEn, m_graticuleEn, m_marginScalesEn;
	QList<QwtPlotScaleItem *> *m_marginScales;
	QColor m_color;
};

} // namespace scopy

#endif // PLOTGRID_H
