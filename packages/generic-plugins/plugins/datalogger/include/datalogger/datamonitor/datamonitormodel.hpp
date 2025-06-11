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

#ifndef DATAMONITORMODEL_H
#define DATAMONITORMODEL_H

#include "unitofmeasurement.hpp"

#include "../scopy-datalogger_export.h"

#include <QColor>
#include <QObject>

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT DataMonitorModel : public QObject
{

	Q_OBJECT
public:
	DataMonitorModel(QObject *parent = nullptr, double defaultScale = 1, double defaultOffset = 0);
	explicit DataMonitorModel(QString name, QColor color, UnitOfMeasurement *unitOfMeasure = nullptr,
				  double defaultScale = 1, double defaultOffset = 0, QObject *parent = nullptr);

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

	double defaultScale() const;

	double offset() const;
	void setOffset(double newOffset);

	double scale() const;
	void setScale(double newScale);

	double defaultOffset() const;

	bool hasScale() const;
	void setHasScale(bool newHasScale);
	bool hasOffset() const;
	void setHasOffset(bool newHasOffset);

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
	const double m_defaultScale;
	const double m_defaultOffset;
	bool m_hasScale = false;
	bool m_hasOffset = false;
	double m_offset = 0;
	double m_scale = 1;
	void checkMinMaxUpdate(double value);
	UnitOfMeasurement *unitOfMeasure;
	Q_PROPERTY(bool hasOffset READ hasOffset CONSTANT FINAL)
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORMODEL_H
