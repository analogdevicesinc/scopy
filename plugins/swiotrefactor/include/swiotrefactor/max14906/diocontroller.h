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

#ifndef SCOPY_DIOCONTROLLER_H
#define SCOPY_DIOCONTROLLER_H

#include <iio.h>
#include "scopy-swiotrefactor_export.h"
#include <QString>
#include <QThread>
#include <QTimer>

#include <utility>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT DioController : public QObject
{
	Q_OBJECT
public:
	explicit DioController(struct iio_context *context_, QString deviceName = "max14906");
	~DioController() override;

	int getChannelCount();

	iio_device *getDevice() const;

	QString getChannelName(unsigned int index);
	QString getChannelType(unsigned int index);

private:
	QString m_deviceName;
	struct iio_context *m_context;
	struct iio_device *m_device;
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_DIOCONTROLLER_H
