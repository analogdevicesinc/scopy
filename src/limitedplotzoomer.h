#ifndef LIMITEDPLOTZOOMER_H
#define LIMITEDPLOTZOOMER_H

#include <qwt_plot_zoomer.h>

namespace adiscope {
class LimitedPlotZoomer : public QwtPlotZoomer
{
public:
	LimitedPlotZoomer(QWidget*, bool doReplot = false);
	void resetZoom();

protected:
        virtual void zoom(const QRectF &);
};
}

#endif // LIMITEDPLOTZOOMER_H
