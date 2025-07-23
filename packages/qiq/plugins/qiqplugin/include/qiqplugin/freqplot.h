#ifndef FREQPLOT_H
#define FREQPLOT_H

#include "iplot.h"
#include <gui/plotwidget.h>
namespace scopy::qiqplugin {

class FreqPlot : public IPlot
{
public:
	FreqPlot();
	~FreqPlot();

	int id() override;
	void init(QIQPlotInfo info, int samplingFreq) override;
	void updateData(QList<CurveData> curveData) override;
	void setSamplingFreq(int samplingFreq) override;
	QWidget *widget() override;

private:
	void computeXAxis(int size);
	void initAxis();

	int m_samplingFreq;
	qiqplugin::QIQPlotInfo m_plotInfo;
	PlotWidget *m_plot = nullptr;
	QVector<double> m_xFreq;
};

} // namespace scopy::qiqplugin

#endif // FREQPLOT_H
