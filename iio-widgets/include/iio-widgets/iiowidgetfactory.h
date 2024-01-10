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
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetFactory : public QWidget
{
	Q_OBJECT
public:
	enum // Attribute factory hint, might have to move to smth like scopy::AFH
	{
		AutoHint = 0x00, // auto hint is 0x0, might change to NoHint

		TimeSave = 0x000001,
		InstantSave = 0x000002,
		ExternalSave = 0x000004,

		EditableUi = 0x000008,
		ComboUi = 0x000010,
		SwitchUi = 0x00020,
		RangeUi = 0x000040,

		AttrData = 0x000080,
		TriggerData = 0x000100,
		DeviceAttrData = 0x000200,
		FileDemoData = 0x000400,

		ProgressLineEditUi = 0x000800,
	};

	explicit IIOWidgetFactory(QWidget *parent = nullptr);
	~IIOWidgetFactory();

	IIOWidget *buildSingle(uint32_t hint = AutoHint, IIOWidgetFactoryRecipe recipe = {});
	QList<IIOWidget *> buildAllAttrsForChannel(struct iio_channel *channel);
	QList<IIOWidget *> buildAllAttrsForDevice(struct iio_device *dev);

private:
	struct iio_channel *m_channel;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETFACTORY_H
