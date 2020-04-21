#ifndef NYQUISTPLOTZOOMER_H
#define NYQUISTPLOTZOOMER_H

#include <qwt_polar_magnifier.h>

namespace adiscope {
class NyquistPlotZoomer : public QwtPolarMagnifier {
public:
	NyquistPlotZoomer(QwtPolarCanvas *parent);

public:
	void zoomIn();
	void zoomOut();
	bool isZoomed();
	void cancelZoom();

private:
	int zoom_count;
};
} // namespace adiscope

#endif // NYQUISTPLOTZOOMER_H
