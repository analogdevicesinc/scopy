#ifndef FFTPLOTMANAGER_H
#define FFTPLOTMANAGER_H
#include "scopy-adc_export.h"
#include <QWidget>
#include <interfaces.h>
#include <channelcomponent.h>
#include <measurementpanel.h>
#include "plotmanager.h"
#include "fftplotcomponent.h"

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT FFTPlotManager : public PlotManager
{
	Q_OBJECT
public:
	FFTPlotManager(QString name = "FFTPlotManager", QWidget *parent = nullptr);
	~FFTPlotManager();

	virtual uint32_t addPlot(QString name) override;
	virtual void removePlot(uint32_t uuid) override;
	FFTPlotComponent *plot(uint32_t uuid);

private:
	PlotComponent *m_primary;
	void multiPlotUpdate();

	void syncNavigatorAndCursors(PlotComponent *);
	void syncAllPlotNavigatorsAndCursors();
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTMANAGER_H
