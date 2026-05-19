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

#ifndef ARMGPIOWIDGET_H
#define ARMGPIOWIDGET_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <iio.h>
#include <iio-widgets/iiowidget.h>
#include <menuonoffswitch.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT ArmGpioWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ArmGpioWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~ArmGpioWidget();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void onEnableAckChanged();

private:
	void setupUi();
	QWidget *createGpioConfigSection(QWidget *parent);
	QWidget *createEnableAckSection(QWidget *parent);
	void readEnableAckFromDevice();
	void writeEnableAckToDevice();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	QList<IIOWidget *> m_widgets;

	// Enable ACK bit 4 checkboxes (CHECKBOX_MASK pattern)
	QList<MenuOnOffSwitch *> m_enableAckBit4;
	// Attribute names for enable-ack (to read-modify-write bit 4)
	QStringList m_enableAckAttrNames;
};

} // namespace scopy::ad9371

#endif // ARMGPIOWIDGET_H
