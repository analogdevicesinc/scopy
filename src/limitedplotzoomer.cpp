#include "limitedplotzoomer.h"

#include <qwt_plot.h>

using namespace adiscope;

LimitedPlotZoomer::LimitedPlotZoomer(QWidget *parent, bool doReplot)
	: QwtPlotZoomer(parent, doReplot) {
	setMaxStackDepth(5);
}

void LimitedPlotZoomer::resetZoom() { QwtPlotZoomer::zoom(0); }

void LimitedPlotZoomer::zoom(const QRectF &rect) {
	QRectF boundedRect = rect & zoomBase();

	QwtPlotZoomer::zoom(boundedRect);
}
