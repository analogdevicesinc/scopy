#ifndef DATAMONITORMODEL_H
#define DATAMONITORMODEL_H

#include "unitofmeasurement.hpp"

#include <QColor>
#include <QObject>

namespace scopy {
namespace datamonitor {

class DataMonitorModel : public QObject
{

	Q_OBJECT
public:
	explicit DataMonitorModel(QString title, QColor color, UnitOfMeasurement *unitOfMeasure,
				  QObject *parent = nullptr);

	QString getTitle() const;
	void setTitle(QString newTitle);

	QColor getColor() const;
	void setColor(QColor newColor);

	UnitOfMeasurement *getUnitOfMeasure() const;
	void setUnitOfMeasure(UnitOfMeasurement *newUnitOfMeasure);

	double getLastReadValue() const;
	double getValueAtTime(uint32_t time);
	void updateValue(uint32_t time, double value);
	void resetMinMax();

	QList<QPair<uint32_t, double>> *getValues() const;

Q_SIGNALS:
	void valueUpdated(uint32_t time, double value);
	void minValueUpdated(double value);
	void maxValueUpdated(double value);

private:
	QString title;
	QColor color;
	UnitOfMeasurement *unitOfMeasure;
	double m_lastReadValue;
	double m_minValue;
	double m_maxValue;
	// a list of  <time , value > pairs
	QList<QPair<uint32_t, double>> *values;
	void checkMinMaxUpdate(double value);
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORMODEL_H
