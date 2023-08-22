#ifndef MEASUREMENTPANEL_H
#define MEASUREMENTPANEL_H

#include <QWidget>
#include "widgets/measurementlabel.h"
#include <QScrollBar>
#include "scopy-gui_export.h"

namespace Ui {
class MeasurementsPanel;
};

namespace scopy::gui {

class SCOPY_GUI_EXPORT MeasurementPanel : public QWidget
{
public:
	MeasurementPanel(QWidget *parent = nullptr);
	QWidget *cursorArea();

public slots:
	void addMeasurement(MeasurementLabel *meas);
	void removeMeasurement(MeasurementLabel *meas);
	void update();
private:
	Ui::MeasurementsPanel *ui;
};
}
#endif // MEASUREMENTPANEL_H
