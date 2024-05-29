#ifndef MEASURECOMPONENT_H
#define MEASURECOMPONENT_H
#include "toolcomponent.h"
#include "gui/widgets/measurementsettings.h"
#include "gui/widgets/menucontrolbutton.h"
#include "scopy-adcplugin_export.h"
#include "interfaces.h"

namespace scopy {
namespace adc {

class SCOPY_ADCPLUGIN_EXPORT MeasureComponent : public QObject, public ToolComponent
{
public:
	MeasureComponent(ToolTemplate *tool, MeasurementPanelInterface* p, QObject *parent);
	MeasurementSettings *measureSettings();

private:
	void setupMeasureButtonHelper(MenuControlButton*);
	MeasurementSettings *m_measureSettings;

	QString measureMenuId = "measure";
	QString statsMenuId = "stats";
	MenuControlButton *measure;
	MeasurementPanelInterface* m_measurementPanelInterface;
};
}
}

#endif // MEASURECOMPONENT_H
