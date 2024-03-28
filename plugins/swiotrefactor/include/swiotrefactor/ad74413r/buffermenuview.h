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

#ifndef SWIOTGENERICMENU_H
#define SWIOTGENERICMENU_H

#include "buffermenu.h"

#include <QMap>
#include <QObject>
#include <QWidget>

#include <iio.h>

namespace scopy::swiotrefactor {
class BufferMenuView : public QWidget
{
	Q_OBJECT
public:
	BufferMenuView(QMap<QString, iio_channel *> chnls, Connection *conn, QWidget *parent = nullptr);
	~BufferMenuView();

	void init(QString title, QString function, QPen color, QString unit, double yMin, double yMax);

	BufferMenu *getAdvMenu();

Q_SIGNALS:
	void setYMin(double yMin);
	void minChanged(double yMin);
	void setYMax(double yMax);
	void maxChanged(double yMax);

	void diagnosticFunctionUpdated();
	void samplingFrequencyUpdated(int chnlId);
	void broadcastThresholdForward();
	void broadcastThresholdBackward();

private:
	void createConnections();
	QWidget *createVerticalSettingsMenu(QString unit, double yMin, double yMax, QWidget *parent);

	BufferMenu *m_swiotAdvMenu;
	Connection *m_connection;
	QMap<QString, iio_channel *> m_chnls;
};
} // namespace scopy::swiotrefactor

#endif // SWIOTGENERICMENU_H
