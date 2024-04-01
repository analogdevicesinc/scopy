#ifndef DATAMONITORMODEL_H
#define DATAMONITORMODEL_H

#include "ireadstrategy.hpp"
#include "unitofmeasurement.hpp"

#include "scopy-datamonitorplugin_export.h"

#include <QColor>
#include <QObject>

namespace scopy {
namespace datamonitor {

class SCOPY_DATAMONITORPLUGIN_EXPORT DataMonitorModel : public QObject
{

	Q_OBJECT
public:
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
	void updateValue(double time, double value);
	void resetMinMax();

	QList<QPair<double, double>> *getValues() const;
	void clearMonitorData();

	QVector<double> *getXdata();
	QVector<double> *getYdata();

	IReadStrategy *getReadStrategy() const;
	void setReadStrategy(IReadStrategy *newReadStrategy);
	void read();

	double minValue() const;
	double maxValue() const;

	QString getShortName() const;
	void setShortName(const QString &newShortName);

	QString getDeviceName() const;
	void setDeviceName(const QString &newDeviceName);

Q_SIGNALS:
	void valueUpdated(double time, double value);
	void minValueUpdated(double value);
	void maxValueUpdated(double value);
	void dataCleared();

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
	void setDataStorageSize();

	IReadStrategy *readStrategy;
	UnitOfMeasurement *unitOfMeasure;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORMODEL_H
