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

#ifndef AD936X_HELPER_H
#define AD936X_HELPER_H

#include "scopy-ad936x_export.h"
#include <QWidget>
#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy {
namespace ad936x {

class SCOPY_AD936X_EXPORT AD936xHelper : public QWidget
{
	Q_OBJECT
public:
	AD936xHelper(QWidget *parent = nullptr);

	QWidget *generateGlobalSettingsWidget(iio_device *dev, QString title, QWidget *parent);

	QWidget *generateRxDeviceWidget(iio_device *dev, QString title, QWidget *parent);
	QWidget *generateRxChannelWidget(iio_channel *chn, QString title, QWidget *parent);

	QWidget *generateTxDeviceWidget(iio_device *dev, QString title, QWidget *parent);
	QWidget *generateTxChannelWidget(iio_channel *chn, QString title, QWidget *parent);

Q_SIGNALS:
	void readRequested();
};
} // namespace ad936x
} // namespace scopy
#endif // AD936X_HELPER_H
