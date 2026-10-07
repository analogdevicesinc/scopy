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

#include "digitaliomath.h"

#include <QWidget>

#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>

class QLabel;
class QSlider;
class QStackedWidget;
class QTimer;

namespace scopy {
class MenuCombo;

namespace gui {
class MenuSpinbox;
}

namespace adalm2000 {
class DioPinWidget;
class M2kDigitalIoController;

class DioGroupWidget : public QWidget
{
	Q_OBJECT
public:
	DioGroupWidget(M2kDigitalIoController *controller, IIOWidgetGroup *group, int groupIndex,
		       QWidget *parent = nullptr);
	~DioGroupWidget() override;

	void setInputState(quint16 gpi, quint16 shorted);

	bool isGrouped() const;
	void setGrouped(bool grouped);

	DioPinWidget *pinWidget(int pin) const;
	IIOWidget *groupDirectionWidget() const { return m_directionWidget; }
	IIOWidget *groupValueWidget() const { return m_valueWidget; }

private:
	void onModeChanged(int index);
	void onGroupDirectionChanged(const QString &value);
	void valueUpdated(int value);

	M2kDigitalIoController *m_controller;
	IIOWidgetGroup *m_group;
	int m_groupIndex;

	MenuCombo *m_modeCombo = nullptr;
	QStackedWidget *m_stack = nullptr;
	IIOWidget *m_directionWidget = nullptr;
	IIOWidget *m_valueWidget = nullptr;
	QSlider *m_slider = nullptr;
	gui::MenuSpinbox *m_valueSpin = nullptr;
	QTimer *m_sliderWriteTimer = nullptr;
	DioPinWidget *m_pins[dio::PINS_PER_GROUP] = {};

	// Owns the slider's position: the value attribute in output mode, the poll in
	// input mode. Never both.
	bool m_groupIsOutput = false;
};

} // namespace adalm2000
} // namespace scopy
