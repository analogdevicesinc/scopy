#ifndef DATAMONITORMODEL_H
#define DATAMONITORMODEL_H

#include "unitofmeasurement.hpp"

#include "../scopy-dataloggerplugin_export.h"

#include <QColor>
#include <QObject>

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT DataMonitorModel : public QObject
{

	Q_OBJECT
public:
	DataMonitorModel(QObject *parent = nullptr);
	explicit DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
				  QObject *parent = nullptr);

	QString getName() const;
	void setName(QString newName);

	QColor getColor() const;
	void setColor(QColor newColor);

	UnitOfMeasurement *getUnitOfMeasure() const;
	void setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure);

	QPair<double, double> getLastReadValue() const;
	double getValueAtTime(double time);
	void setValueAtTime(double time, double value);
	void addValue(double time, double value);

	void clearMonitorData();

	QVector<double> *getXdata();
	QVector<double> *getYdata();

	double minValue() const;
	double maxValue() const;
	void resetMinMax();

	QString getShortName() const;
	void setShortName(const QString &newShortName);

	QString getDeviceName() const;
	void setDeviceName(const QString &newDeviceName);

	void setYdata(const QVector<double> &newYdata);

	void setXdata(const QVector<double> &newXdata);

	void setMinValue(double newMinValue);

	void setMaxValue(double newMaxValue);

Q_SIGNALS:
	void valueUpdated(double time, double value);
	void minValueUpdated(double value);
	void maxValueUpdated(double value);
	void dataCleared();

protected:
	void setDataStorageSize();

private:
	QString name;
	QString shortName;
	QString deviceName;
	QColor color;
	double m_minValue;
	double m_maxValue;
	double m_dataSize;
	QVector<double> ydata;
	QVector<double> xdata;
	void checkMinMaxUpdate(double value);
	UnitOfMeasurement *unitOfMeasure;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORMODEL_H
