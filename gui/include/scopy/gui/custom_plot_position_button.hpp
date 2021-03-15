/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CUSTOMPLOTPOSITIONBUTTON_H
#define CUSTOMPLOTPOSITIONBUTTON_H

#include <QButtonGroup>
#include <QWidget>

#include <scopy/gui/colored_qwidget.hpp>

namespace Ui {
class CustomPlotPositionButton;
}

namespace scopy {
namespace gui {

class CustomPlotPositionButton : public ColoredQWidget
{
	Q_OBJECT

public:
	enum ReadoutsPosition
	{
		topLeft,
		topRight,
		bottomLeft,
		bottomRight
	};

	explicit CustomPlotPositionButton(QWidget* parent = 0);
	~CustomPlotPositionButton();
	void setPosition(ReadoutsPosition position);

Q_SIGNALS:
	void positionChanged(CustomPlotPositionButton::ReadoutsPosition position);

private:
	Ui::CustomPlotPositionButton* m_ui;
	QButtonGroup* m_btns;
};
} // namespace gui
} // namespace scopy

#endif // CUSTOMPLOTPOSITIONBUTTON_H
