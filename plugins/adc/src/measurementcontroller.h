/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef MEASUREMENTCONTROLLER_H
#define MEASUREMENTCONTROLLER_H

#include "measure.h"
#include "scopy-adc_export.h"

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

class SCOPY_ADC_EXPORT MeasurementController : public QObject
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

class SCOPY_ADC_EXPORT TimeChannelMeasurementController : public MeasurementController
{
public:
	TimeChannelMeasurementController(TimeMeasureModel *msr, QPen m_pen, QObject *parent = nullptr);
};

class SCOPY_ADC_EXPORT MeasureManagerInterface : public QObject
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

class SCOPY_ADC_EXPORT TimeMeasureManager : public MeasureManagerInterface
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

} // namespace scopy::adc
#endif // MEASUREMENTCONTROLLER_H
