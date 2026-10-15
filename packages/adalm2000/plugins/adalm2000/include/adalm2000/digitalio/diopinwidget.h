/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include <QWidget>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>

class QFrame;
class QLabel;

namespace scopy {
namespace component {
class Attribute;
}

namespace adalm2000 {
class M2kDigitalIoController;

class DioPinWidget : public QWidget
{
	Q_OBJECT
public:
	DioPinWidget(M2kDigitalIoController *controller, IIOWidgetGroup *group, int pin, QWidget *parent = nullptr);
	~DioPinWidget() override;

	int pin() const { return m_pin; }

	// `shorted` means an output pin disagrees with what it is driving.
	void setInputState(bool high, bool shorted);

	IIOWidget *directionWidget() const { return m_directionWidget; }
	IIOWidget *valueWidget() const { return m_valueWidget; }

private:
	IIOWidget *buildToggle(component::Attribute *attr, bool isDirection, QWidget *parent);
	// Must be re-applied on every redisplay: CustomSwitch::update() pins its own
	// maximum size back.
	void stretchToggle(IIOWidget *w);

	int m_pin;
	IIOWidgetGroup *m_group;
	QLabel *m_label = nullptr;
	QFrame *m_led = nullptr;
	IIOWidget *m_directionWidget = nullptr;
	IIOWidget *m_valueWidget = nullptr;
};

} // namespace adalm2000
} // namespace scopy
