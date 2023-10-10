#ifndef MEASUREMENTSETTINGS_H
#define MEASUREMENTSETTINGS_H

#include "scopy-gui_export.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

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

Q_SIGNALS:
	void toggleAllMeasurements(bool);
	void toggleAllStats(bool);
	void sortMeasurements(MeasurementSortingType type);
	void sortStats(MeasurementSortingType type);
	void enableMeasurementPanel(bool b);
	void enableStatsPanel(bool b);
};
} // namespace scopy

#endif // MEASUREMENTSETTINGS_H
