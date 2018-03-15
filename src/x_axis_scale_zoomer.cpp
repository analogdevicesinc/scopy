#include "x_axis_scale_zoomer.h"

using namespace adiscope;

XAxisScaleZoomer::XAxisScaleZoomer(QWidget *parent):
	OscScaleZoomer(parent)
{
}

XAxisScaleZoomer::~XAxisScaleZoomer()
{
}

void XAxisScaleZoomer::zoom(const QRectF& rect)
{
	QRectF boundedRect = rect & zoomBase();
	QRectF baseRect = zoomBase();

	boundedRect.setTop(baseRect.top());
	boundedRect.setBottom(baseRect.bottom());

	QwtPlotZoomer::zoom(boundedRect);

}
