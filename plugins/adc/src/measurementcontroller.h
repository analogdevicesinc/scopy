#ifndef MEASUREMENTCONTROLLER_H
#define MEASUREMENTCONTROLLER_H

#include "measure.h"
#include "scopy-adcplugin_export.h"

#include <QObject>

#include <widgets/measurementlabel.h>
#include <widgets/measurementpanel.h>

namespace scopy::adc {
class GRTimeChannelAddon;

typedef struct
{
	QString name;
	QString icon;
	QString unit;
	QString formatter;
	QString type;
} MeasurementInfo;

class SCOPY_ADCPLUGIN_EXPORT MeasurementController : public QObject
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

class SCOPY_ADCPLUGIN_EXPORT TimeChannelMeasurementController : public MeasurementController
{
public:
	TimeChannelMeasurementController(TimeMeasureModel *msr, QPen m_pen, QObject *parent = nullptr);
};

class SCOPY_ADCPLUGIN_EXPORT MeasureManagerInterface : public QObject
{
	Q_OBJECT
Q_SIGNALS:
	void enableMeasurement(MeasurementLabel *);
	void disableMeasurement(MeasurementLabel *);
	void toggleAllMeasurement(bool b);
	void toggleAllStats(bool b);
	void enableStat(StatsLabel *);
	void disableStat(StatsLabel *);

public:
	virtual void initMeasure(QPen) = 0;
	virtual QWidget *createMeasurementMenu(QWidget *parent) = 0;
	virtual MeasurementController *getController() = 0;
	virtual MeasureModel *getModel() = 0;
};

class SCOPY_ADCPLUGIN_EXPORT TimeMeasureManager : public MeasureManagerInterface
{
public:
	TimeMeasureManager(QObject *parent = nullptr);
	~TimeMeasureManager();

public:
	virtual void initMeasure(QPen);
	virtual QWidget *createMeasurementMenu(QWidget *parent);
	virtual MeasurementController *getController();
	virtual MeasureModel *getModel();

protected:
	virtual QWidget *createMeasurementMenuSection(QString category, QWidget *parent);

	TimeChannelMeasurementController *m_measureController;
	TimeMeasureModel *m_measureModel;
};

} // namespace scopy::grutil
#endif // MEASUREMENTCONTROLLER_H
