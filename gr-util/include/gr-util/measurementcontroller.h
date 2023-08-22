#ifndef MEASUREMENTCONTROLLER_H
#define MEASUREMENTCONTROLLER_H

#include <QObject>
#include <widgets/measurementpanel.h>
#include "measure.h"
#include "scopy-gr-util_export.h"

namespace scopy::grutil {
class GRTimeChannelAddon;

typedef struct  {
	QString name;
	QString icon;
	QString unit;
	QString formatter;
	QString type;
} MeasurementInfo;

class SCOPY_GR_UTIL_EXPORT MeasurementController : public QObject
{
	Q_OBJECT
public:
	MeasurementController(QPen pen, MeasureModel *msr, QObject *parent = nullptr);
	virtual ~MeasurementController();
	
	MeasurementLabel* findMeasurementLabel(QString name);
	virtual MeasurementLabel* enableMeasurement(QString name);
	virtual void disableMeasurement(QString name);
	void addMeasurement(MeasurementInfo v);
	QList<MeasurementInfo> availableMeasurements() const;

Q_SIGNALS:
	void measurementEnabled(MeasurementLabel*);
	void measurementDisabled(MeasurementLabel*);

protected:
	QList<MeasurementInfo> m_availableMeasurements;
	MeasureModel *m_measure;

private:
	QPen m_pen;
	QList<MeasurementLabel*> m_labels;

};

class SCOPY_GR_UTIL_EXPORT TimeChannelMeasurementController : public MeasurementController {
public:
	TimeChannelMeasurementController(TimeMeasureModel* msr, QPen m_pen, QObject *parent = nullptr);
};

}
#endif // MEASUREMENTCONTROLLER_H
