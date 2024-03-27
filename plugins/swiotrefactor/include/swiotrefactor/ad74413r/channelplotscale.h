/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef CHANNELPLOTSCALE_H
#define CHANNELPLOTSCALE_H

#include <QLabel>
#include <QWidget>

#include <gui/plot_utils.hpp>

namespace scopy::swiotrefactor {
class ChannelPlotScale : public QWidget
{
	Q_OBJECT
public:
	explicit ChannelPlotScale(int channel, QString unit, QColor m_color, QWidget *parent = nullptr);
	~ChannelPlotScale();

	void setInstantValue(double value);
	void setUnitPerDivision(double value);
	int getChannelId();
	bool getEnabled();
	void setEnabled(bool en);
Q_SIGNALS:
	void requestUpdate();

private Q_SLOTS:
	void update();

private:
	QString m_unit;
	QLabel *m_unitPerDivisionLbl;
	QLabel *m_instantValueLbl;
	QColor m_channelColor;
	int m_channel;
	double m_unitPerDivision;
	double m_instantValue;
	bool m_enabled;
	MetricPrefixFormatter *m_formatter;
};
} // namespace scopy::swiotrefactor

#endif // CHANNELPLOTSCALE_H
