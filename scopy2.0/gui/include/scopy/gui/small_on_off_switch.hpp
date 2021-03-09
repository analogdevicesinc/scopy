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

#ifndef SMALL_ON_OFF_SWITCH_HPP
#define SMALL_ON_OFF_SWITCH_HPP

#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <scopy/gui/custom_animation.hpp>

class QShowEvent;

namespace scopy {
namespace gui {

class SmallOnOffSwitch : public QPushButton
{
	Q_OBJECT

	Q_PROPERTY(int m_durationMs MEMBER m_durationMs WRITE setDuration);

	Q_PROPERTY(QColor m_colorStart MEMBER m_colorStart);
	Q_PROPERTY(QColor m_colorEnd MEMBER m_colorEnd);
	Q_PROPERTY(QColor m_color MEMBER m_color WRITE setHandleColor);

public:
	explicit SmallOnOffSwitch(QWidget* parent = nullptr);
	~SmallOnOffSwitch();

private:
	QColor m_colorStart, m_colorEnd, m_color;
	QLabel m_on, m_off;
	QWidget m_handle;
	CustomAnimation m_anim;
	CustomAnimation m_colorAnim;
	QString m_stylesheet;
	int m_durationMs;
	bool m_showIcon;
	bool m_bothValid;

	void setDuration(int ms);
	void setHandleColor(const QColor& m_color);
	void updateOnOffLabels();

	bool event(QEvent* e);
	void showEvent(QShowEvent* event);
	void paintEvent(QPaintEvent*);

private Q_SLOTS:
	void toggleAnim(bool enabled);
Q_SIGNALS:
	void animationDone();
};
} // namespace gui
} // namespace scopy

#endif /* SMALL_ON_OFF_SWITCH_HPP */
