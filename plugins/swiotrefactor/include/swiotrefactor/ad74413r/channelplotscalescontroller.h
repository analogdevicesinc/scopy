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

#ifndef CHANNELPLOTSCALESCONTROLLER_H
#define CHANNELPLOTSCALESCONTROLLER_H

#include "channelplotscale.h"

#include <QHBoxLayout>
#include <QMap>
#include <QWidget>

#include <gui/plot_utils.hpp>

namespace scopy::swiotrefactor {
class ChannelPlotScalesController : public QWidget
{
	Q_OBJECT
public:
	explicit ChannelPlotScalesController(QWidget *parent);
	~ChannelPlotScalesController();

	void addChannel(int index, QColor color, QString unit, bool enabled);
	void setChannelEnabled(int channel, bool enabled);
	void setUnitPerDivision(int channel, double unitPerDivision);
	void setInstantValue(int channel, double value);

private:
	void updateLayout();

private:
	QString m_unit;
	QColor *m_channelColor;
	QVector<ChannelPlotScale *> m_channelPlotScales;
	QHBoxLayout *m_layout;
};
} // namespace scopy::swiotrefactor

#endif // CHANNELPLOTSCALESCONTROLLER_H
