#include "nyquistplotzoomer.h"


using namespace adiscope;

NyquistPlotZoomer::NyquistPlotZoomer(QwtPolarCanvas* parent):
    QwtPolarMagnifier(parent), zoom_count(0)
{

}

void NyquistPlotZoomer::zoomIn()
{
	QwtPolarMagnifier::rescale(0.8);
	++zoom_count;
}

void NyquistPlotZoomer::zoomOut()
{
	QwtPolarMagnifier::rescale(1.25);
	if (zoom_count > 0)
		--zoom_count;
}

bool NyquistPlotZoomer::isZoomed()
{
	return (zoom_count != 0);
}

void NyquistPlotZoomer::cancelZoom()
{
	zoom_count = 0;
	QwtPolarMagnifier::unzoom();
}
