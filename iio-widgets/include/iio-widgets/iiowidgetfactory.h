/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef SCOPY_IIOWIDGETFACTORY_H
#define SCOPY_IIOWIDGETFACTORY_H

#include <QWidget>
#include <QLoggingCategory>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <iio.h>
#include "iiowidgetdata.h"
#include "iiowidget.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetFactory : public QObject
{
	Q_OBJECT
public:
	enum // Attribute factory hint
	{
		AutoHint = 0x00, // auto hint is 0x0, might change to NoHint

		EditableUi = 0x000001,
		ComboUi = 0x000002,
		SwitchUi = 0x00004,
		RangeUi = 0x000008,

		AttrData = 0x000010,
		TriggerData = 0x000020,
		DeviceAttrData = 0x000040,
		ContextAttrData = 0x000080,

		CMDQAttrData = 0x000100,
		CMDQDeviceAttrData = 0x000200,
	};

	explicit IIOWidgetFactory(QObject *parent = nullptr);
	~IIOWidgetFactory();

	static IIOWidget *buildSingle(uint32_t hint, IIOWidgetFactoryRecipe recipe, QWidget *parent = nullptr);
	static QList<IIOWidget *> buildAllAttrsForChannel(struct iio_channel *channel, QWidget *parent = nullptr);
	static QList<IIOWidget *> buildAllAttrsForDevice(struct iio_device *dev, QWidget *parent = nullptr);
	static QList<IIOWidget *> buildAllAttrsForContext(struct iio_context *context, QWidget *parent = nullptr);
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETFACTORY_H
