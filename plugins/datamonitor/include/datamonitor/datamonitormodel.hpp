#ifndef DATAMONITORMODEL_H
#define DATAMONITORMODEL_H

#include "ireadstrategy.hpp"
#include "unitofmeasurement.hpp"

#include "scopy-datamonitor_export.h"

#include <QColor>
#include <QObject>

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITOR_EXPORT DataMonitorModel : public QObject
{

	Q_OBJECT
public:
	explicit DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure,
				  QObject *parent = nullptr);

	QString getName() const;
	void setName(QString newName);

	QColor getColor() const;
	void setColor(QColor newColor);

	UnitOfMeasurement *getUnitOfMeasure() const;
	void setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure);

	QPair<double,double> getLastReadValue() const;
	double getValueAtTime(double time);
	void updateValue(double time, double value);
	void resetMinMax();

	QList<QPair<double, double>> *getValues() const;
	void clearMonitorData();

	IReadStrategy *getReadStrategy() const;
	void setReadStrategy(IReadStrategy *newReadStrategy);
	void read();

Q_SIGNALS:
	void valueUpdated(double time, double value);
	void minValueUpdated(double value);
	void maxValueUpdated(double value);
	void dataCleared();

private:
	QString name;
	QColor color;
	UnitOfMeasurement *unitOfMeasure;
	double m_minValue;
	double m_maxValue;
	QVector<double> ydata;
	QVector<double> xdata;
	void checkMinMaxUpdate(double value);
	IReadStrategy *readStrategy;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORMODEL_H
