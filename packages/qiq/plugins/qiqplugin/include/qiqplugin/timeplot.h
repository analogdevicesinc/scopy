#ifndef TIMEPLOT_H
#define TIMEPLOT_H

#include "iplot.h"
#include <gui/plotwidget.h>
namespace scopy::qiqplugin {

class TimePlot : public IPlot
{
public:
	TimePlot();
	~TimePlot();

	int id() override;
	void init(QIQPlotInfo info, int samplingFreq) override;
	void updateData(QList<CurveData> curveData) override;
	void setSamplingFreq(int samplingFreq) override;
	QWidget *widget() override;

private:
	void computeXAxis(int size);
	void initAxis();

	int m_samplingFreq;
	PlotWidget *m_plot = nullptr;
	qiqplugin::QIQPlotInfo m_plotInfo;
	QVector<double> m_xTime;
};

} // namespace scopy::qiqplugin

#endif // TIMEPLOT_H
