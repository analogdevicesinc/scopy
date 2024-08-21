#ifndef MEASUREMENTSETTINGS_H
#define MEASUREMENTSETTINGS_H

#include "menusectionwidget.h"
#include "scopy-gui_export.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <menuonoffswitch.h>

namespace scopy {
class SCOPY_GUI_EXPORT MeasurementSettings : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		MPM_SORT_CHANNEL,
		MPM_SORT_TYPE
	} MeasurementSortingType;
	MeasurementSettings(QWidget *parent = nullptr);
	~MeasurementSettings();

	bool measurementEnabled();
	bool statsEnabled();
	bool markerEnabled();

	MenuSectionWidget *getMarkerSection() const;

	MenuSectionWidget *getStatsSection() const;

	MenuSectionWidget *getMeasureSection() const;

Q_SIGNALS:
	void toggleAllMeasurements(bool);
	void toggleAllStats(bool);
	void sortMeasurements(MeasurementSortingType type);
	void sortStats(MeasurementSortingType type);
	void enableMeasurementPanel(bool b);
	void enableStatsPanel(bool b);
	void enableMarkerPanel(bool b);

private:
	MenuOnOffSwitch *measurePanelSwitch;
	MenuOnOffSwitch *statsPanelSwitch;
	MenuOnOffSwitch *markerPanelSwitch;

	MenuSectionWidget *markerSection;
	MenuSectionWidget *statsSection;
	MenuSectionWidget *measureSection;
};
} // namespace scopy

#endif // MEASUREMENTSETTINGS_H
