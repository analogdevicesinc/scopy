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
 */

#ifndef AGCSETUPWIDGET_H
#define AGCSETUPWIDGET_H

#include <QWidget>
#include <iio.h>
#include <gui/widgets/menusectionwidget.h>

namespace scopy::adrv9009 {

class AgcSetupWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AgcSetupWidget(iio_device *device, QWidget *parent = nullptr);
	~AgcSetupWidget();

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();
	QWidget *createAgcConfigurationWidget(QWidget *parent);
	QWidget *createAnalogPeakDetector(QWidget *parent);
	QWidget *createPowerMeasurementDetector(QWidget *parent);
	QWidget *createAgcRxChannelGroup(const QString &baseAttr, const QString &displayName, const QString &range,
					 QWidget *parent);

	iio_device *m_device;
};

} // namespace scopy::adrv9009

#endif // AGCSETUPWIDGET_H
