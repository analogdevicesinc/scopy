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
 *
 */

#ifndef LEDBUTTON_H
#define LEDBUTTON_H

#include <QPushButton>
#include <QTimer>

namespace scopy {
class LedButton : public QPushButton
{
	Q_OBJECT
public:
	LedButton(QWidget *parent = nullptr);
	~LedButton();

	void setLedState(bool ledState);
	void ledOn();
	void ledOff();

private:
	QTimer *m_timer;
};
} // namespace scopy

#endif // LEDBUTTON_H
