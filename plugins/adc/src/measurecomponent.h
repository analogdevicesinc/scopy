#ifndef MEASURECOMPONENT_H
#define MEASURECOMPONENT_H
#include "toolcomponent.h"
#include "gui/widgets/measurementpanel.h"
#include "gui/widgets/measurementsettings.h"
#include "gui/widgets/menucontrolbutton.h"

namespace scopy {
namespace adc {

class MeasureComponent : public QObject, public ToolComponent
{
public:
	MeasureComponent(ToolTemplate *tool, QObject *parent);
	MeasurementSettings *measureSettings();
	StatsPanel *statsPanel();
	MeasurementsPanel *measurePanel();

private:
	void setupMeasureButtonHelper(MenuControlButton*);
	MeasurementsPanel *m_measurePanel;
	MeasurementSettings *m_measureSettings;
	StatsPanel *m_statsPanel;

	QString measureMenuId = "measure";
	QString statsMenuId = "stats";
};
}
}

#endif // MEASURECOMPONENT_H
