#ifndef PLOTFACTORY_H
#define PLOTFACTORY_H

#include <freqplot.h>
#include <qiqutils.h>
#include <iplot.h>
#include <timeplot.h>
#include <xyplot.h>

namespace scopy::qiqplugin {
class PlotFactory
{
public:
	static IPlot *createPlot(QIQPlotInfo::PlotType type)
	{
		switch(type) {
		case QIQPlotInfo::PlotType::TIME:
			return new TimePlot();
		case QIQPlotInfo::PlotType::FREQ:
			return new FreqPlot();
		case QIQPlotInfo::PlotType::XY:
			return new XYPlot();
		default:
			return nullptr;
		}
	}
};
} // namespace scopy::qiqplugin

#endif // PLOTFACTORY_H
