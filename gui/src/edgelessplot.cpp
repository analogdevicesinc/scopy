#include "edgelessplot.h"

using namespace scopy;

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv &from_scaleDiv);

static QwtScaleDiv getEdgelessScaleDiv(const QwtScaleDiv &from_scaleDiv)
{
	double lowerBound;
	double upperBound;
	QList<double> minorTicks;
	QList<double> mediumTicks;
	QList<double> majorTicks;

	lowerBound = from_scaleDiv.lowerBound();
	upperBound = from_scaleDiv.upperBound();
	minorTicks = from_scaleDiv.ticks(QwtScaleDiv::MinorTick);
	mediumTicks = from_scaleDiv.ticks(QwtScaleDiv::MediumTick);
	majorTicks = from_scaleDiv.ticks(QwtScaleDiv::MajorTick);
	if(majorTicks.size() >= 2) {
		majorTicks.erase(majorTicks.begin());
		majorTicks.erase(majorTicks.end() - 1);
	}
	return QwtScaleDiv(lowerBound, upperBound, minorTicks, mediumTicks, majorTicks);
}
/*
 * EdgelessPlotScaleItem class implementation
 */
EdgelessPlotScaleItem::EdgelessPlotScaleItem(QwtScaleDraw::Alignment alignment, const double pos)
	: QwtPlotScaleItem(alignment, pos)
{}

void EdgelessPlotScaleItem::updateScaleDiv(const QwtScaleDiv &xScaleDiv, const QwtScaleDiv &yScaleDiv)
{
	QwtPlotScaleItem::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv), getEdgelessScaleDiv(yScaleDiv));
}

/*
 * EdgelessPlotGrid class implementation
 */
EdgelessPlotGrid::EdgelessPlotGrid()
	: QwtPlotGrid()
{}

void EdgelessPlotGrid::updateScaleDiv(const QwtScaleDiv &xScaleDiv, const QwtScaleDiv &yScaleDiv)
{
	QwtPlotGrid::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv), getEdgelessScaleDiv(yScaleDiv));
}
