#ifndef EXTENDINGPLOTZOOMER_H
#define EXTENDINGPLOTZOOMER_H

#include "limitedplotzoomer.h"

#include <qwt_plot_shapeitem.h>

namespace adiscope {
class ExtendingPlotZoomer : public LimitedPlotZoomer {
public:
	explicit ExtendingPlotZoomer(QWidget *, bool doReplot = false);
	virtual ~ExtendingPlotZoomer();

protected:
	virtual QPolygon adjustedPoints(const QPolygon &) const;
	virtual void zoom(const QRectF &);

private:
	mutable bool widthPass;
	mutable bool heightPass;
	mutable bool extendWidth;
	mutable bool extendHeight;

	QVector<QwtPlotShapeItem *> cornerMarkers;
	QVector<QwtPlotShapeItem *> extendMarkers;
};
} // namespace adiscope

#endif // EXTENDINGPLOTZOOMER_H
