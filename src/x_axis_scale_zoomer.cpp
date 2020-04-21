#include "x_axis_scale_zoomer.h"

#include "qwt_scale_draw.h"

#include <dbgraph.hpp>

using namespace adiscope;

XAxisScaleZoomer::XAxisScaleZoomer(QWidget *parent) : OscScaleZoomer(parent) {}

XAxisScaleZoomer::~XAxisScaleZoomer() {}

void XAxisScaleZoomer::zoom(const QRectF &rect) {
	QRectF boundedRect = rect & zoomBase();
	QRectF baseRect = zoomBase();

	boundedRect.setTop(baseRect.top());
	boundedRect.setBottom(baseRect.bottom());

	QwtPlotZoomer::zoom(boundedRect);
}

QwtText XAxisScaleZoomer::trackerText(const QPoint &p) const {
	QwtText t;
	QPointF dp = QwtPlotZoomer::invTransform(p);

	const dBgraph *plt = dynamic_cast<const dBgraph *>(plot());
	t.setText(plt->getScaleValueFormat(dp.x(), QwtPlot::xTop, 4) + ", " +
		  plt->getScaleValueFormat(dp.y(), QwtPlot::yLeft));
	return t;
}
