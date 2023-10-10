#ifndef MEASUREMENTCONTROLLER_H
#define MEASUREMENTCONTROLLER_H

#include "measure.h"
#include "scopy-gr-util_export.h"

#include <QObject>

#include <widgets/measurementlabel.h>
#include <widgets/measurementpanel.h>

namespace scopy::grutil {
class GRTimeChannelAddon;

typedef struct
{
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

	MeasurementLabel *findMeasurementLabel(QString name);
	StatsLabel *findStatsLabel(QString name);
	virtual QWidget *enableMeasurement(QString name);
	virtual void disableMeasurement(QString name);

	virtual QWidget *enableStats(QString name);
	virtual void disableStats(QString name);

	void addMeasurement(MeasurementInfo v);
	QList<MeasurementInfo> availableMeasurements() const;

Q_SIGNALS:
	void measurementEnabled(MeasurementLabel *);
	void measurementDisabled(MeasurementLabel *);
	void statsEnabled(StatsLabel *);
	void statsDisabled(StatsLabel *);

protected:
	QList<MeasurementInfo> m_availableMeasurements;
	MeasureModel *m_measure;

private:
	QPen m_pen;
	QList<StatsLabel *> m_statsLabels;
	QList<MeasurementLabel *> m_measureLabels;
};

class SCOPY_GR_UTIL_EXPORT TimeChannelMeasurementController : public MeasurementController
{
public:
	TimeChannelMeasurementController(TimeMeasureModel *msr, QPen m_pen, QObject *parent = nullptr);
};

} // namespace scopy::grutil
#endif // MEASUREMENTCONTROLLER_H
