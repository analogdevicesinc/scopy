/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 *
 */

#ifndef AUXADCDACIOWIDGET_H
#define AUXADCDACIOWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <iio.h>
#include <menuonoffswitch.h>
#include <iiowidgetbuilder.h>

namespace scopy {
namespace pluto {

class AuxAdcDacIoWidget : public QWidget
{
	Q_OBJECT
public:
	explicit AuxAdcDacIoWidget(QString uri, QWidget *parent = nullptr);
	~AuxAdcDacIoWidget();

signals:

private:
	QString m_uri;
	QVBoxLayout *m_layout;
	iio_device *m_device;
	QWidget *tempSensorWidget(QWidget *parent);
	QWidget *auxAdcWidget(QWidget *parent);
	QWidget *auxDacWidget(QWidget *parent);
	QWidget *getAuxDac(QString dacx, QWidget *parent);
	QWidget *controlsOutWidget(QWidget *parent);
	QWidget *gposWidget(QWidget *parent);
	QWidget *gpoWidget(QString gpox, QWidget *parent);

	MenuOnOffSwitch *m_gpo0Mask;
	MenuOnOffSwitch *m_gpo1Mask;
	MenuOnOffSwitch *m_gpo2Mask;
	MenuOnOffSwitch *m_gpo3Mask;
	void applyGpoMask();
};
} // namespace pluto
} // namespace scopy

#endif // AUXADCDACIOWIDGET_H
