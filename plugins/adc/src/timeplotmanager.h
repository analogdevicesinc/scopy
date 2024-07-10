#ifndef TIMEPLOTMANAGER_H
#define TIMEPLOTMANAGER_H
#include "scopy-adc_export.h"
#include <QWidget>
#include <interfaces.h>
#include <channelcomponent.h>
#include <measurementpanel.h>
#include "plotmanager.h"

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotManager : public PlotManager {
public:
	TimePlotManager(QString name = "TimePlotManager", QWidget *parent = nullptr);
	~TimePlotManager();

	virtual uint32_t addPlot(QString name) override;
	virtual void removePlot(uint32_t uuid) override;
	TimePlotComponent *plot(uint32_t uuid);

private:
	void multiPlotUpdate();
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTMANAGER_H
