#ifndef EDGELESSPLOT_H
#define EDGELESSPLOT_H

#include "scopy-gui_export.h"

#include <QwtPlotGrid>
#include <QwtPlotScaleItem>

namespace scopy {
/*
 * EdgelessPlotScaleItem class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotScaleItem : public QwtPlotScaleItem
{
public:
	explicit EdgelessPlotScaleItem(QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale, const double pos = 0.0);
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};

/*
 * EdgelessPlotGrid class ensures that the first and last major ticks are ignored
 */
class SCOPY_GUI_EXPORT EdgelessPlotGrid : public QwtPlotGrid
{
public:
	explicit EdgelessPlotGrid();
	virtual void updateScaleDiv(const QwtScaleDiv &, const QwtScaleDiv &);
};
} // namespace scopy

#endif // EDGELESSPLOT_H
