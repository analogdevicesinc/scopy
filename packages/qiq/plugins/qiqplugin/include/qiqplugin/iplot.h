#ifndef IPLOT_H
#define IPLOT_H

#include "qiqutils.h"

#include <QWidget>
#include <qiqcontroller/qiqplotinfo.h>
namespace scopy::qiqplugin {

class IPlot : public QObject
{
	Q_OBJECT
public:
	virtual int id() = 0;
	virtual void init(QIQPlotInfo info, int samplingFreq) = 0;
	virtual void updateData(QList<CurveData> curveData) = 0;
	virtual void setSamplingFreq(int samplingFreq) = 0;
	virtual QWidget *widget() = 0;
	virtual ~IPlot() = default;
};

} // namespace scopy::qiqplugin

#endif // IPLOT_H
