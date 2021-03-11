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
#include "measurement_gui.h"
#include "measure.h"

#include <QLabel>

using namespace adiscope;

/*
 * Class MeasurementGui implementation
 */
MeasurementGui::MeasurementGui():
	m_name(""),
	m_value(""),
	m_nameLabel(NULL),
	m_valueLabel(NULL),
	m_minValLableWidth(0)
{
}

MeasurementGui::~MeasurementGui()
{
}

QString MeasurementGui::name() const
{
	return m_name;
}

QString MeasurementGui::value() const
{
	return m_value;
}

void MeasurementGui::setLabelsColor(const QColor& color)
{
	QString stylesheet = QString("color: %1;").arg(color.name());
	if (m_nameLabel)
		m_nameLabel->setStyleSheet(stylesheet);
	if (m_valueLabel)
		m_valueLabel->setStyleSheet(stylesheet);
}

void MeasurementGui::init(QLabel *name, QLabel *value)
{
	m_nameLabel = name;
	m_valueLabel = value;
}


/*
 * Class MetricMeasurementGui implementation
 */

 MetricMeasurementGui::MetricMeasurementGui():
		MeasurementGui()
{
}

void MetricMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 KHz");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	MeasurementGui::init(name, value);
}

void MetricMeasurementGui::update(const MeasurementData& data, double displayScale)
{
	m_name = data.name() + ":";
	if (data.measured() && data.enabled()) {
		double value = data.value();
		if (data.axis() == MeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value = m_formatter.format(value, data.unit(), 3);
	}
	else
		m_value = "--";

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class TimeMeasurementGui implementation
 */

 TimeMeasurementGui::TimeMeasurementGui():
		MeasurementGui()
{
}

void TimeMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 ms");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	MeasurementGui::init(name, value);
}

void TimeMeasurementGui::update(const MeasurementData& data, double displayScale)
{
	m_name = data.name() + ":";

	if (data.measured() && data.enabled()) {
		double value = data.value();
		if (data.axis() == MeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value = m_formatter.format(value, "", 3);
	} else
		m_value = "--";

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class PercentageMeasurementGui implementation
 */

 PercentageMeasurementGui::PercentageMeasurementGui():
		MeasurementGui()
{
}

void PercentageMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("100.00 %");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	MeasurementGui::init(name, value);
}

void PercentageMeasurementGui::update(const MeasurementData& data, double displayScale)
{
	m_name = data.name() + ":";

	if (data.measured() && data.enabled()) {
		double value = data.value();
		if (data.axis() == MeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value.setNum(value, 'f', 2);
		m_value += "%";
	} else {
		m_value = "--";
	}

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class DimensionlessMeasurementGui implementation
 */

 DimensionlessMeasurementGui::DimensionlessMeasurementGui():
		MeasurementGui()
{
}

void DimensionlessMeasurementGui::update(const MeasurementData& data, double displayScale)
{
	m_name = data.name() + ":";

	if (data.measured() && data.enabled()) {
		double value = data.value();
		if (data.axis() == MeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value.setNum(value, 'f', 3);
	} else
		m_value = "--";

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}
