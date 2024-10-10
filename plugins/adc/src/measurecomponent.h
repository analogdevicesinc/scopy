#ifndef MEASURECOMPONENT_H
#define MEASURECOMPONENT_H
#include "toolcomponent.h"
#include "gui/widgets/measurementsettings.h"
#include "gui/widgets/menucontrolbutton.h"
#include "scopy-adc_export.h"
#include "interfaces.h"

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT MeasureComponent : public QObject, public ToolComponent
{
public:
	MeasureComponent(ToolTemplate *tool, QButtonGroup *btngroup, MeasurementPanelInterface *p, QObject *parent);
	MeasurementSettings *measureSettings();

private:
	void setupMeasureButtonHelper(MenuControlButton *);
	MeasurementSettings *m_measureSettings;
	QButtonGroup *hoverBtnGroup;

	QString measureMenuId = "measure";
	QString statsMenuId = "stats";
	MenuControlButton *measure;
	MeasurementPanelInterface *m_measurementPanelInterface;
};
} // namespace adc
} // namespace scopy

#endif // MEASURECOMPONENT_H
