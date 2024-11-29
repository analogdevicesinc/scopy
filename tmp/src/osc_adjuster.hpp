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

#ifndef M2K_OSC_ADJUSTER_H
#define M2K_OSC_ADJUSTER_H

#include <qobject.h>
#include <QWidget>
#include <qevent.h>
#include <qwt_plot.h>

class QWidget;
class QMouseEvent;
class QWheelEvent;

class OscAdjuster: public QObject
{
	Q_OBJECT
public:
	explicit OscAdjuster(QWidget*, QwtAxisId axisId);
	virtual ~OscAdjuster();

	QWidget *parentWidget();
	const QWidget *parentWidget() const;

	QwtAxisId axisId();

	void setMouseFactor(double);
	double mouseFactor() const;

	void setEnabled(bool on);
	bool isEnabled() const;

	void setMouseButton(Qt::MouseButton button);
	void getMouseButton(Qt::MouseButton &button) const;

	virtual bool eventFilter(QObject *object, QEvent *event);

Q_SIGNALS:
	void wheelUp(int);
	void wheelDown(int);

protected:
	virtual void rescale(double factor) = 0;

	virtual void widgetMousePressEvent(QMouseEvent *mouseEvent);
	virtual void widgetMouseReleaseEvent(QMouseEvent *mouseEvent);
	virtual void widgetMouseMoveEvent(QMouseEvent *mouseEvent) = 0;
	virtual void widgetMouseWheelEvent(QWheelEvent *wheelEvent);

	double d_mouseFactor;
	bool d_mousePressed;
	bool d_hasMouseTracking;
	bool d_isEnabled;
	QPoint d_mousePos;
	Qt::MouseButton d_mouseButton;
	QwtAxisId d_axisId;
};

class VertMouseGestures: public OscAdjuster
{
	Q_OBJECT
public:
	explicit VertMouseGestures(QWidget *parent, QwtAxisId axisId):
		OscAdjuster(parent, axisId)
	{
	}

	virtual ~VertMouseGestures()
	{

	}

Q_SIGNALS:
	void upMovement(double);
	void downMovement(double);

protected:
	virtual void rescale(double factor)
	{
		factor = qAbs( factor );
		if ( factor == 1.0 || factor == 0.0 )
			return;

		if (factor > 1.0)
			Q_EMIT upMovement(factor);
		else
			Q_EMIT downMovement(factor);
	}
	virtual void widgetMouseMoveEvent(QMouseEvent *mouseEvent)
	{
		if (!d_mousePressed)
			return;

		const int dy = mouseEvent->pos().y() - d_mousePos.y();
		if (dy != 0)
		{
			double f = d_mouseFactor;
			if (dy < 0)
				f = 1 / f;

			rescale(f);
		}

		d_mousePos = mouseEvent->pos();
	}
};

class HorizMouseGestures: public OscAdjuster
{
	Q_OBJECT
public:
	explicit HorizMouseGestures(QWidget *parent, QwtAxisId axisId):
		OscAdjuster(parent, axisId)
	{
	}

	virtual ~HorizMouseGestures()
	{

	}

Q_SIGNALS:
	void leftMovement(double);
	void rightMovement(double);

protected:
	virtual void rescale(double factor)
	{
		factor = qAbs( factor );
		if ( factor == 1.0 || factor == 0.0 )
			return;

		if (factor > 1.0)
			Q_EMIT leftMovement(factor);
		else
			Q_EMIT rightMovement(factor);
	}

	virtual void widgetMouseMoveEvent(QMouseEvent *mouseEvent)
	{
		if ( !d_mousePressed )
			return;

		    const int dx = mouseEvent->pos().x() - d_mousePos.x();
		    if ( dx != 0 )
		    {
			double f = d_mouseFactor;
			if ( dx < 0 )
			    f = 1 / f;

			rescale( f );
		    }

		    d_mousePos = mouseEvent->pos();
	}
};

#endif /* M2K_OSC_ADJUSTER_H */
