/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef COMPLETIONCIRCLE_H
#define COMPLETIONCIRCLE_H

#include <QDial>

namespace adiscope {
class CompletionCircle : public QDial
{
	Q_OBJECT

	Q_PROPERTY(bool inverted MEMBER invert_circle);
public:
	explicit CompletionCircle(QWidget *parent = 0,
	                          bool invert_circle = false);

	double valueDouble();
	double minimumDouble();
	double maximumDouble();
	bool toggledState();
	bool isLogScale();

	void setOrigin(double);
	void setMinimumDouble(double);
	void setMaximumDouble(double);
	void setIsLogScale(bool);

public Q_SLOTS:
	void setValueDouble(double);
	void setToggled(bool);

Q_SIGNALS:
	void toggled(bool);

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *me);
	void mouseReleaseEvent(QMouseEvent *me);
	void mouseMoveEvent(QMouseEvent *me);
	void keyPressEvent(QKeyEvent *ev);
	void wheelEvent(QWheelEvent *e);

	bool pointInsideCircle(const QPoint&, int xc, int yc, int r);

	// Hide setter/getter of the int value
	using QAbstractSlider::value;
	using QAbstractSlider::setValue;
	using QAbstractSlider::minimum;
	using QAbstractSlider::setMinimum;
	using QAbstractSlider::maximum;
	using QAbstractSlider::setMaximum;

private:
	double m_double_value;
	double m_double_minimum;
	double m_double_maximum;
	double m_origin;
	int m_xc;
	int m_yc;
	int m_radius;
	bool m_pressed;
	bool m_log_scale;
	bool invert_circle;
};
}

#endif // COMPLETIONCIRCLE_H
