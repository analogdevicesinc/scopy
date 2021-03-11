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

#ifndef COMPLETIONCIRCLE_H
#define COMPLETIONCIRCLE_H

#include <QDial>

namespace scopy {
namespace gui {
class CompletionCircle : public QDial
{
	Q_OBJECT

	Q_PROPERTY(bool inverted MEMBER m_invertCircle);

public:
	explicit CompletionCircle(QWidget* parent = 0, bool m_invertCircle = false);

	double valueDouble();
	double minimumDouble();
	double maximumDouble();
	bool toggledState();
	bool isLogScale();
	bool toggleable();

	void setOrigin(double);
	void setMinimumDouble(double);
	void setMaximumDouble(double);
	void setIsLogScale(bool);
	void setToggleable(bool);

public Q_SLOTS:
	void setValueDouble(double);
	void setToggled(bool);

Q_SIGNALS:
	void toggled(bool);

protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* me);
	void mouseReleaseEvent(QMouseEvent* me);
	void mouseMoveEvent(QMouseEvent* me);
	void keyPressEvent(QKeyEvent* ev);
	void wheelEvent(QWheelEvent* e);

	bool pointInsideCircle(const QPoint&, int xc, int yc, int r);

	// Hide setter/getter of the int value
	using QAbstractSlider::maximum;
	using QAbstractSlider::minimum;
	using QAbstractSlider::setMaximum;
	using QAbstractSlider::setMinimum;
	using QAbstractSlider::setValue;
	using QAbstractSlider::value;

private:
	double m_doubleValue;
	double m_doubleMinimum;
	double m_doubleMaximum;
	double m_origin;
	int m_xc;
	int m_yc;
	int m_radius;
	bool m_pressed;
	bool m_logScale;
	bool m_invertCircle;
	bool m_toggleable;
};
} // namespace gui
} // namespace scopy
#endif // COMPLETIONCIRCLE_H
