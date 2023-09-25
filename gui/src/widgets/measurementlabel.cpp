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
#include "widgets/measurementlabel.h"
#include <QLabel>

using namespace scopy;

/*
 * Class MeasurementLabel implementation
 */

MeasurementLabel::MeasurementLabel(QWidget *parent) : QWidget(parent) {
	QHBoxLayout *lay = new QHBoxLayout(this);
	m_unit = "";
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(lay);
	lay->setMargin(0);
	lay->setSpacing(0);
	m_nameLabel = new QLabel(this);
	setName("None");
	m_valueLabel = new QLabel("---", this);
	m_formatter = nullptr;
	m_precision = 2;

	lay->addWidget(m_nameLabel);
	lay->addSpacerItem(new QSpacerItem(10,0,QSizePolicy::Expanding,QSizePolicy::Maximum));
	lay->addWidget(m_valueLabel);
	m_valueLabel->setAlignment(Qt::AlignRight);

	m_color = StyleHelper::getColor("LabelText");
	StyleHelper::MeasurementPanelLabel(this, m_name + "MeasurementLabel");

}

MeasurementLabel::~MeasurementLabel() {

}

void MeasurementLabel::setName(QString str) {
	m_name = str;
	m_nameLabel->setText(str+" : ");
}

void MeasurementLabel::setUnit(QString str) {
	m_unit = str;
}

void MeasurementLabel::setPrecision(int val) {
	m_precision = val;
}

void MeasurementLabel::setColor(QColor color) {
	m_color = color;
	StyleHelper::MeasurementPanelLabel(this, m_name + "MeasurementLabel");
}

void MeasurementLabel::setValue(double val) {
	if(!m_formatter) {
		m_valueLabel->setText( QString::number(val,'g', m_precision) + " " + m_unit);
	} else {
		m_valueLabel->setText( m_formatter->format(val, m_unit, m_precision));
	}
}

void MeasurementLabel::setMeasurementValueFormatter(PrefixFormatter *f) {
	m_formatter = f;
}

QString MeasurementLabel::name() const
{
	return m_name;
}

QColor MeasurementLabel::color() const
{
	return m_color;
}

int MeasurementLabel::idx() const
{
	return m_idx;
}

void MeasurementLabel::setIdx(int newIdx)
{
	m_idx = newIdx;
}

StatsLabel::StatsLabel(QWidget *parent) : QWidget(parent) {
	QVBoxLayout *lay = new QVBoxLayout(this);

	m_unit = "";
	setLayout(lay);
	lay->setMargin(0);
	m_nameLabel = new QLabel(this);
	setName("None");

	m_avgLabel = new QLabel("Avg: ---", this);
	m_minLabel = new QLabel("Min: ---", this);
	m_maxLabel = new QLabel("Max: ---", this);
	lay->setAlignment(m_nameLabel, Qt::AlignCenter);
	lay->setAlignment(Qt::AlignLeft);
	m_formatter = nullptr;
	m_precision = 2;
	m_idx = 0;

	lay->addWidget(m_nameLabel);
	lay->addWidget(m_avgLabel);
	lay->addWidget(m_minLabel);
	lay->addWidget(m_maxLabel);
	lay->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Maximum,QSizePolicy::Expanding));
}

StatsLabel::~StatsLabel() {

}

void StatsLabel::setName(QString str) {
	m_name = str;
	m_nameLabel->setText(str);
}

void StatsLabel::setUnit(QString str) {
	m_unit = str;
}

void StatsLabel::setPrecision(int val) {
	m_precision = val;
}

void StatsLabel::setColor(QColor color) {
	m_color = color;
	StyleHelper::StatsPanelLabel(this, m_name + "MeasurementLabel");
}

void StatsLabel::setValue(double avg, double min, double max) {
	if(!m_formatter) {
		m_avgLabel->setText( "Avg: " + QString::number(avg,'g', m_precision) + " " + m_unit);
		m_minLabel->setText( "Min: " + QString::number(min,'g', m_precision) + " " + m_unit);
		m_maxLabel->setText( "Max: " + QString::number(max,'g', m_precision) + " " + m_unit);
	} else {
		m_avgLabel->setText( "Avg: " + m_formatter->format(avg, m_unit, m_precision));
		m_minLabel->setText( "Min: " + m_formatter->format(min, m_unit, m_precision));
		m_maxLabel->setText( "Max: " + m_formatter->format(max, m_unit, m_precision));
	}
}

void StatsLabel::setMeasurementValueFormatter(PrefixFormatter *f) {
	m_formatter = f;
}

QString StatsLabel::name() const
{
	return m_name;
}

QColor StatsLabel::color() const
{
	return m_color;
}

int StatsLabel::idx() const
{
	return m_idx;
}

void StatsLabel::setIdx(int newIdx)
{
	m_idx = newIdx;
}

#include "moc_measurementlabel.cpp"
