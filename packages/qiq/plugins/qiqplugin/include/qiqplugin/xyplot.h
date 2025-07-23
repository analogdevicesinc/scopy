#ifndef XYPLOT_H
#define XYPLOT_H

#include "iplot.h"
#include <gui/plotwidget.h>
namespace scopy::qiqplugin {

class XYPlot : public IPlot
{
public:
	XYPlot();
	~XYPlot();

	int id() override;
	void init(QIQPlotInfo info, int samplingFreq) override;
	void updateData(QList<CurveData> curveData) override;
	QWidget *widget() override;
	void setSamplingFreq(int samplingFreq) override;

private:
	void initAxis();

	qiqplugin::QIQPlotInfo m_plotInfo;
	PlotWidget *m_plot = nullptr;
};

} // namespace scopy::qiqplugin

#endif // XYPLOT_H
