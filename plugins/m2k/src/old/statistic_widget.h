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

#ifndef STATISTIC_WIDGET_H
#define STATISTIC_WIDGET_H

#include <QLabel>

namespace Ui {
	class Statistic;
}

namespace scopy {

class MeasurementData;
class Statistic;
class Formatter;

class StatisticWidget: public QWidget
{
public:
	explicit StatisticWidget(QWidget *parent = nullptr);
	~StatisticWidget();

	QString title() const;
	int channelId() const;
	int positionIndex() const;

	void setTitleColor(const QColor& color);
	void setPositionIndex(int pos);

	void initForMeasurement(const MeasurementData & data);
	void updateStatistics(const Statistic & data);

private:
	Ui::Statistic *m_ui;
	QString m_title;
	int m_channelId;
	int m_posIndex;
	Formatter *m_formatter;
	int m_valueLabelWidth;
};

} // namespace scopy

#endif // STATISTIC_WIDGET_H
