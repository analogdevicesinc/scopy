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

#include "statistic_widget.h"
#include "gui/measure.h"
#include "plot_utils.hpp"
#include "ui_statistic.h"

namespace scopy {
class Formatter
{
public:
	Formatter()
	{
	}

	virtual ~Formatter()
	{
	}

	virtual QString format(double value) const = 0;
};

class MetricFormatter: public Formatter
{
public:
	MetricFormatter(QString unit = ""):
	m_unit(unit)
	{
	}

	void setUnit(QString unit)
	{
		m_unit = unit;
	}

	QString format(double value) const
	{
		return m_formatter.format(value, m_unit, 3);
	}

private:
	MetricPrefixFormatter m_formatter;
	QString m_unit;

};

class TimeFormatter: public Formatter
{
public:
	TimeFormatter()
	{
	}

	QString format(double value) const
	{
		return m_formatter.format(value, "", 3);
	}

private:
	TimePrefixFormatter m_formatter;
};

class PercentageFormatter: public Formatter
{
public:
	PercentageFormatter()
	{
	}

	QString format(double value) const
	{
		QString text;

		text.setNum(value, 'f', 2);
		text += "%";

		return text;
	}
};

class DimensionlessFormatter: public Formatter
{
public:
	DimensionlessFormatter()
	{
	}

	QString format(double value) const
	{
		QString text;

		text.setNum(value, 'f', 3);

		return text;
	}
};

}

using namespace scopy;

StatisticWidget::StatisticWidget(QWidget *parent):
	QWidget(parent),
	m_ui(new Ui::Statistic),
	m_title(""),
	m_channelId(-1),
	m_posIndex(-1),
	m_formatter(new DimensionlessFormatter()),
	m_valueLabelWidth(0)
{
	m_ui->setupUi(this);
}

StatisticWidget::~StatisticWidget()
{
	delete m_formatter;
	delete m_ui;
}

QString StatisticWidget::title() const
{
	return m_title;
}

int StatisticWidget::channelId() const
{
	return m_channelId;
}

void StatisticWidget::setTitleColor(const QColor& color)
{
	QString stylesheet = QString(""
		"font-size: 14px;"
		"font-weight: bold;"
		"color: %1;").arg(color.name());

	m_ui->label_count->setStyleSheet(stylesheet);
	m_ui->label_title->setStyleSheet(stylesheet);
}

void StatisticWidget::setPositionIndex(int pos)
{
	if (m_posIndex != pos) {
		m_posIndex = pos;
		QString posText;
		posText.setNum(pos);
		m_ui->label_count->setText(posText);
	}
}

void StatisticWidget::initForMeasurement(const MeasurementData &data)
{
	m_title = data.name();
	m_channelId = data.channel();

	m_ui->label_title->setText(m_title);

	delete m_formatter;
	m_formatter = nullptr;

	QLabel *label = new QLabel(m_ui->label_avg);

	switch(data.unitType()) {
	case MeasurementData::METRIC:
		m_formatter = new MetricFormatter(data.unit());

		label->setText("-999.999 mV");
		m_valueLabelWidth = label->minimumSizeHint().width();

		break;
	case MeasurementData::TIME:
		m_formatter = new TimeFormatter();

		label->setText("-999.999 ms");
		m_valueLabelWidth = label->minimumSizeHint().width();

		break;
	case MeasurementData::PERCENTAGE:
		m_formatter = new PercentageFormatter();

		label->setText("100.00 %");
		m_valueLabelWidth = label->minimumSizeHint().width();

		break;
	default:
		m_formatter = new DimensionlessFormatter();

		label->setText("-999.999");
		m_valueLabelWidth = label->minimumSizeHint().width();

		break;
	}

	m_ui->label_avg->setMinimumWidth(m_valueLabelWidth);
	m_ui->label_min->setMinimumWidth(m_valueLabelWidth);
	m_ui->label_max->setMinimumWidth(m_valueLabelWidth);

	delete label;
}

void StatisticWidget::updateStatistics(const Statistic& data)
{
	QString avg_text;
	QString min_text;
	QString max_text;

	if (data.numPushedData() == 0) {
		avg_text = "--";
		min_text = "--";
		max_text = "--";
	} else {
		avg_text = m_formatter->format(data.average());
		min_text = m_formatter->format(data.min());
		max_text = m_formatter->format(data.max());
	}

	m_ui->label_avg->setText(avg_text);
	m_ui->label_min->setText(min_text);
	m_ui->label_max->setText(max_text);
}
