#ifndef PLOTAXISHANDLE_H
#define PLOTAXISHANDLE_H
#include "plotwidget.h"
#include "scopy-gui_export.h"
#include <axishandle.h>
#include <QObject>

namespace scopy {
class AxisHandle;

class SCOPY_GUI_EXPORT PlotAxisHandle : public QWidget
{
	Q_OBJECT
public:
	PlotAxisHandle(PlotWidget *plot, PlotAxis *ax);
	~PlotAxisHandle();

	void setAxis(PlotAxis *axis);
	PlotAxis *axis() const;
	AxisHandle *handle() const;

	double getPosition() const;
	void setPosition(double pos);
	void setPositionSilent(double pos);

	double pixelToScale(int pos);
	int scaleToPixel(double pos);

	void init();
	void deinit();

Q_SIGNALS:
	void scalePosChanged(double);
	void updatePos();

private:
	double m_pos;
	PlotWidget *m_plotWidget;
	PlotAxis *m_axis;
	QwtPlot *m_plot;
	AxisHandle *m_handle;
};
} // namespace scopy

#endif // PLOTAXISHANDLE_H
