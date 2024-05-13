#ifndef MEASURECOMPONENT_H
#define MEASURECOMPONENT_H
#include "toolcomponent.h"
#include "gui/widgets/measurementsettings.h"
#include "gui/widgets/menucontrolbutton.h"
#include "plotcomponent.h"
#include "scopy-adcplugin_export.h"

namespace scopy {
namespace adc {

class SCOPY_ADCPLUGIN_EXPORT MeasureComponent : public QObject, public ToolComponent
{
public:
	MeasureComponent(ToolTemplate *tool, PlotComponentManager* p, QObject *parent);
	MeasurementSettings *measureSettings();
	/*void addPlotComponent(PlotComponent *c);
	void removePlotComponent(PlotComponent *c);*/

private:
	void setupMeasureButtonHelper(MenuControlButton*);
	MeasurementSettings *m_measureSettings;

	QString measureMenuId = "measure";
	QString statsMenuId = "stats";
	MenuControlButton *measure;
	PlotComponentManager* m_plotComponentManager;
};
}
}

#endif // MEASURECOMPONENT_H
