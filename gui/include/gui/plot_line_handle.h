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

#ifndef PLOT_LINE_HANDLE_H
#define PLOT_LINE_HANDLE_H

#include <QWidget>
#include <QPen>
#include <plot_utils.hpp>
#include "scopygui_export.h"

class HandlesArea;

class SCOPYGUI_EXPORT PlotLineHandle: public QWidget
{
	Q_OBJECT

	friend class HandlesArea;
	friend class GateHandlesArea;

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
	void reset();

protected:
	void enterEvent(QEvent *event);
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

class SCOPYGUI_EXPORT PlotGateHandle: public PlotLineHandle{
public:
	PlotGateHandle(const QPixmap &handleIcon, QWidget *parent = 0);
	void triggerMove();
	void setPosition(int);
	void setPositionSilenty(int);
	void updatePosition();
	void moveWithinParent(int x, int y);
	void setInnerSpacing(int value);
	int position();
	void setTimeValue(double val);
	void setCenterLeft(bool val);
	int getCurrentPos();
	bool reachedLimit();
	void setOtherCursorPosition(int position);
protected:
	void paintEvent(QPaintEvent *event);
	int originPosToCenter(int origin);
	int centerPosToOrigin(int center);
private:
	int m_position;
	int m_otherCursorPos;
	bool m_reachLimit;
	bool m_alignLeft;
	double m_timeValue;
	scopy::TimePrefixFormatter d_timeFormatter;
};

class SCOPYGUI_EXPORT PlotLineHandleH: public PlotLineHandle
{
public:
	PlotLineHandleH(const QPixmap &handleIcon, QWidget *parent = 0,
			bool facingBottom = false);
	void triggerMove();
	void setPosition(int);
	void setPositionSilenty(int);
	void updatePosition();
	void moveWithinParent(int x, int y);
	void setInnerSpacing(int value);

protected:
	void paintEvent(QPaintEvent *event);
	int originPosToCenter(int origin);
	int centerPosToOrigin(int center);
	void mouseDoubleClickEvent(QMouseEvent *event);

protected:
	bool m_facingBottom;
};

class SCOPYGUI_EXPORT PlotLineHandleV: public PlotLineHandle
{
public:
	PlotLineHandleV(const QPixmap &handleIcon, QWidget *parent = 0,
			bool facingRight = false);
	void triggerMove();
	void setPosition(int);
	void setPositionSilenty(int);
	void updatePosition();
	void moveWithinParent(int x, int y);

protected:
	void paintEvent(QPaintEvent *event);
	int originPosToCenter(int origin);
	int centerPosToOrigin(int center);
	void mouseDoubleClickEvent(QMouseEvent *event);

protected:
	bool m_facingRight;
};

class SCOPYGUI_EXPORT FreePlotLineHandleH: public PlotLineHandleH
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

class SCOPYGUI_EXPORT FreePlotLineHandleV: public PlotLineHandleV
{
public:

	FreePlotLineHandleV(const QPixmap &handleIcon,
			const QPixmap &beyondTopIcon,
			const QPixmap &beyondBottomIcon,
			QWidget *parent = 0, bool facingRight = false,
			const QString &name = {});
	void moveWithinParent(int x, int y);

	void setName(const QString &name);
	QString getName() const;

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_beyondTopImage;
	QPixmap m_beyondBottomImage;
	bool m_isBeyondTop;
	bool m_isBeyondBottom;
	QString m_name;
};

class SCOPYGUI_EXPORT RoundedHandleV: public FreePlotLineHandleV
{
	Q_OBJECT
public:
	RoundedHandleV(const QPixmap &handleIcon,
		       const QPixmap &beyondTopIcon,
		       const QPixmap &beyondBottomIcon,
		       QWidget *parent = 0, bool facingRight = false,
		       const QString& name = {},
		       bool selectable = false);

	QColor roundRectColor();
	void setRoundRectColor(const QColor &);

	void setSelected(bool selected);
	bool isSelected() const;

	void setSelectable(bool selectable);

Q_SIGNALS:
	void selected(bool);

protected:
	void paintEvent(QPaintEvent *event);

	void mouseDoubleClickEvent(QMouseEvent *event);

private:
	QColor m_roundRectColor;
	bool m_selected;
	bool m_selectable;
};

#endif // PLOT_LINE_HANDLE_H
