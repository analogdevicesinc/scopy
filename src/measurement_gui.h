#ifndef MEASUREMENT_GUI_H
#define MEASUREMENT_GUI_H

#include "plot_utils.hpp"
#include <QString>

class QLabel;

namespace adiscope {

class MeasurementData;

class MeasurementGui
{
public:
	MeasurementGui();
	virtual ~MeasurementGui();

	QString name() const;
	QString value() const;
	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data) = 0;

protected:
	QString m_name;
	QString m_value;
	QLabel *m_nameLabel;
	QLabel *m_valueLabel;
	int m_minValLableWidth;
};

class MetricMeasurementGui: public MeasurementGui
{
public:
	MetricMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data);

protected:
	MetricPrefixFormatter m_formatter;
};

class TimeMeasurementGui: public MeasurementGui
{
public:
	TimeMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data);

protected:
	TimePrefixFormatter m_formatter;
};

class PercentageMeasurementGui: public MeasurementGui
{
public:
	PercentageMeasurementGui();

	virtual void init(QLabel *name, QLabel *value);
	virtual void update(const MeasurementData& data);
};

class DimensionlessMeasurementGui: public MeasurementGui
{
public:
	DimensionlessMeasurementGui();

	virtual void update(const MeasurementData& data);
};

} // namespace adiscope

#endif // MEASUREMENT_GUI_H
