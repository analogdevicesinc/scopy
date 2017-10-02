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

#ifndef PLOT_LINE_HANDLE_H
#define PLOT_LINE_HANDLE_H

#include <QWidget>
#include <QPen>

class HandlesArea;

class PlotLineHandle: public QWidget
{
	Q_OBJECT

	friend class HandlesArea;

public:
	PlotLineHandle(const QPixmap &handleIcon, QWidget *parent = 0);

	void moveSilently(QPoint);
	virtual void moveWithinParent(int x, int y) = 0;
	virtual void setPosition(int) = 0;
	int position();
	void setPen(const QPen&);
	const QPen& pen();

Q_SIGNALS:
	void positionChanged(int);
	void grabbedChanged(bool);
	void mouseReleased();

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

protected:
	bool m_enable_silent_move;

	int m_width;
	int m_height;
	int m_innerSpacing;
	int m_outerSpacing;
	QPixmap m_image;
	QPen m_pen;
	bool m_grabbed;
	int m_current_pos;

private:
	void setGrabbed(bool);
};

class PlotLineHandleH: public PlotLineHandle
{
public:
	PlotLineHandleH(const QPixmap &handleIcon, QWidget *parent = 0,
			bool facingBottom = false);
	void triggerMove();
	void setPosition(int);
	void setPositionSilenty(int);
	void moveWithinParent(int x, int y);
	void setInnerSpacing(int value);

protected:
	void paintEvent(QPaintEvent *event);
	int originPosToCenter(int origin);
	int centerPosToOrigin(int center);

protected:
	bool m_facingBottom;
};

class PlotLineHandleV: public PlotLineHandle
{
public:
	PlotLineHandleV(const QPixmap &handleIcon, QWidget *parent = 0,
			bool facingRight = false);
	void triggerMove();
	void setPosition(int);
	void setPositionSilenty(int);
	void moveWithinParent(int x, int y);

protected:
	void paintEvent(QPaintEvent *event);
	int originPosToCenter(int origin);
	int centerPosToOrigin(int center);

protected:
	bool m_facingRight;
};

class FreePlotLineHandleH: public PlotLineHandleH
{
public:

	FreePlotLineHandleH(const QPixmap &handleIcon,
			const QPixmap &beyondLeftIcon,
			const QPixmap &beyondRightIcon,
			QWidget *parent = 0, bool facingRight = false);
	void moveWithinParent(int x, int y);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_beyondLeftImage;
	QPixmap m_beyondRightImage;
	bool m_isBeyondRight;
	bool m_isBeyondLeft;
};

class FreePlotLineHandleV: public PlotLineHandleV
{
public:

	FreePlotLineHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent = 0, bool facingRight = false);
	void moveWithinParent(int x, int y);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_beyondTopImage;
	QPixmap m_beyondBottomImage;
	bool m_isBeyondTop;
	bool m_isBeyondBottom;
};

class RoundedHandleV: public FreePlotLineHandleV
{
public:
	RoundedHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent = 0, bool facingRight = false);

	QColor roundRectColor();
	void setRoundRectColor(const QColor &);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QColor m_roundRectColor;
};

#endif // PLOT_LINE_HANDLE_H
