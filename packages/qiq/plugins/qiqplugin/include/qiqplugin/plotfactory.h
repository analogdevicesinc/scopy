#ifndef PLOTFACTORY_H
#define PLOTFACTORY_H

#include "timeplot.h"
#include <qiqutils.h>
#include <iplot.h>
#include <plotwidget.h>

namespace scopy::qiqplugin {
class PlotFactory
{
public:
	static IPlot *createPlot(QIQPlotInfo::PlotType type)
	{
		switch(type) {
		case QIQPlotInfo::PlotType::PLOT_WIDGET:
			return new TimePlot();
		default:
			return nullptr;
		}
		return nullptr;
	}
};
} // namespace scopy::qiqplugin

#endif // PLOTFACTORY_H
