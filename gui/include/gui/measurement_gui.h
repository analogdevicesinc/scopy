/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEASUREMENT_GUI_H
#define MEASUREMENT_GUI_H

#include "plot_utils.hpp"
#include <QString>

class QLabel;
class QColor;

namespace scopy {

class MeasurementData;

class SCOPY_GUI_EXPORT MeasurementGui
{
public:
	MeasurementGui();
	virtual ~MeasurementGui();

	QString name() const;
	QString value() const;
	void setLabelsColor(const QColor& color);
	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data, double displayScale) = 0;

protected:
	QString m_name;
	QString m_value;
	QLabel *m_nameLabel;
	QLabel *m_valueLabel;
	int m_minValLableWidth;
};

class SCOPY_GUI_EXPORT MetricMeasurementGui: public MeasurementGui
{
public:
	MetricMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data, double displayScale);

protected:
	MetricPrefixFormatter m_formatter;
};

class SCOPY_GUI_EXPORT TimeMeasurementGui: public MeasurementGui
{
public:
	TimeMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data, double displayScale);

protected:
	TimePrefixFormatter m_formatter;
};

class SCOPY_GUI_EXPORT PercentageMeasurementGui: public MeasurementGui
{
public:
	PercentageMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data, double displayScale);
};

class SCOPY_GUI_EXPORT DecibelsMeasurementGui: public MeasurementGui
{
public:
    DecibelsMeasurementGui();

    virtual void init(QLabel *name, QLabel *value);
    virtual void update(const MeasurementData& data, double displayScale);
};

class SCOPY_GUI_EXPORT DecibelstoCarrierMeasurementGui: public MeasurementGui
{
public:
    DecibelstoCarrierMeasurementGui();

    virtual void init(QLabel *name, QLabel *value);
    virtual void update(const MeasurementData& data, double displayScale);
};

class SCOPY_GUI_EXPORT DimensionlessMeasurementGui: public MeasurementGui
{
public:
	DimensionlessMeasurementGui();

	virtual void update(const MeasurementData& data, double displayScale);
};

} // namespace scopy

#endif // MEASUREMENT_GUI_H