#ifndef MEASUREMENTSELECTOR_H
#define MEASUREMENTSELECTOR_H

#include <QWidget>
#include <QCheckBox>
#include "scopy-gui_export.h"
#include <QVBoxLayout>
#include <QMap>

namespace scopy {


class SCOPY_GUI_EXPORT MeasurementSelectorItem : public QWidget {
	Q_OBJECT
public:
	MeasurementSelectorItem(QString name, QString icon, QWidget *parent);
	~MeasurementSelectorItem();

	QCheckBox *measureCheckbox() const;
	QCheckBox *statsCheckbox() const;

private:
	QCheckBox *m_measureCheckbox;
	QCheckBox *m_statsCheckbox;

};

class SCOPY_GUI_EXPORT MeasurementSelector : public QWidget
{
	Q_OBJECT
public:
	MeasurementSelector(QWidget *parent);
	~MeasurementSelector();

	void addMeasurement(QString name, QString icon);
	void removeMeasurement(QString name);
	MeasurementSelectorItem* measurement(QString name);
private:
	QMap<QString, MeasurementSelectorItem*> m_map;
	QVBoxLayout *lay;

};
}

#endif // MEASUREMENTSELECTOR_H
