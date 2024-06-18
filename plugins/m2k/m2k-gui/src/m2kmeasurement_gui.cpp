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
#include "m2kmeasurement_gui.h"

#include "m2kmeasure.h"

#include <QLabel>

using namespace scopy;

/*
 * Class MeasurementGui implementation
 */
M2kMeasurementGui::M2kMeasurementGui()
	: m_name("")
	, m_value("")
	, m_nameLabel(NULL)
	, m_valueLabel(NULL)
	, m_minValLableWidth(0)
{}

M2kMeasurementGui::~M2kMeasurementGui() {}

QString M2kMeasurementGui::name() const { return m_name; }

QString M2kMeasurementGui::value() const { return m_value; }

void M2kMeasurementGui::setLabelsColor(const QColor &color)
{
	QString stylesheet = QString("color: %1;").arg(color.name());
	if(m_nameLabel)
		m_nameLabel->setStyleSheet(stylesheet);
	if(m_valueLabel)
		m_valueLabel->setStyleSheet(stylesheet);
}

void M2kMeasurementGui::init(QLabel *name, QLabel *value)
{
	m_nameLabel = name;
	m_valueLabel = value;
}

/*
 * Class MetricMeasurementGui implementation
 */

M2kMetricMeasurementGui::M2kMetricMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kMetricMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 KHz");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	M2kMeasurementGui::init(name, value);
}

void M2kMetricMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";
	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value = m_formatter.format(value, data.unit(), 3);
	} else
		m_value = "--";

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class TimeMeasurementGui implementation
 */

M2kTimeMeasurementGui::M2kTimeMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kTimeMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 ms");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	M2kMeasurementGui::init(name, value);
}

void M2kTimeMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";

	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL) {
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

M2kPercentageMeasurementGui::M2kPercentageMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kPercentageMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("100.00 %");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	M2kMeasurementGui::init(name, value);
}

void M2kPercentageMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";

	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL) {
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
 * Class DecibelsMeasurementGui implementation
 */

M2kDecibelsMeasurementGui::M2kDecibelsMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kDecibelsMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 dB");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	M2kMeasurementGui::init(name, value);
}

void M2kDecibelsMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";

	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL_F) {
			value *= displayScale;
		}
		m_value.setNum(value, 'f', 3);
		m_value += "dB";
	} else {
		m_value = "--";
	}

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class DecibelstoCarrierMeasurementGui implementation
 */

M2kDecibelstoCarrierMeasurementGui::M2kDecibelstoCarrierMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kDecibelstoCarrierMeasurementGui::init(QLabel *name, QLabel *value)
{
	// Get the necessary label width so that the label will never resize
	QLabel *label = new QLabel(value);
	label->setText("-999.999 dB");
	m_minValLableWidth = label->minimumSizeHint().width();
	value->setMinimumWidth(m_minValLableWidth);
	delete label;

	M2kMeasurementGui::init(name, value);
}

void M2kDecibelstoCarrierMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";

	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL_F) {
			value *= displayScale;
		}
		m_value.setNum(value, 'f', 3);
		m_value += "dB";
	} else {
		m_value = "--";
	}

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}

/*
 * Class DimensionlessMeasurementGui implementation
 */

M2kDimensionlessMeasurementGui::M2kDimensionlessMeasurementGui()
	: M2kMeasurementGui()
{}

void M2kDimensionlessMeasurementGui::update(const M2kMeasurementData &data, double displayScale)
{
	m_name = data.name() + ":";

	if(data.measured() && data.enabled()) {
		double value = data.value();
		if(data.axis() == M2kMeasurementData::VERTICAL) {
			value *= displayScale;
		}
		m_value.setNum(value, 'f', 3);
	} else
		m_value = "--";

	m_nameLabel->setText(m_name);
	m_valueLabel->setText(m_value);
}
