#ifndef TIMEPLOTMANAGER_H
#define TIMEPLOTMANAGER_H
#include "plotbufferpreviewer.h"
#include "scopy-adc_export.h"
#include <QWidget>
#include "adcinterfaces.h"
#include <measurementpanel.h>
#include "time/timeplotcomponent.h"
#include <gui/channelcomponent.h>
#include <gui/plotmanager.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotManager : public PlotManager
{
	Q_OBJECT
public:
	TimePlotManager(QString name = "TimePlotManager", QWidget *parent = nullptr);
	~TimePlotManager();

	virtual uint32_t addPlot(QString name) override;
	virtual void removePlot(uint32_t uuid) override;
	TimePlotComponent *plot(uint32_t uuid);

private:
	PlotComponent *m_primary;
	PlotBufferPreviewer *m_plotpreviewer;
	AnalogBufferPreviewer *m_bufferpreviewer;
	void multiPlotUpdate();

	// void syncCursors();
	void syncNavigatorAndCursors(PlotComponent *);
	void syncAllPlotNavigatorsAndCursors();
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTMANAGER_H
