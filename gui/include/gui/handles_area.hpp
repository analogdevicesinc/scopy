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

#ifndef HANDLES_AREA_H
#define HANDLES_AREA_H

#include <QWidget>
#include <memory>
#include <vector>

#include "handlesareaextension.h"
#include "scopygui_export.h"

class QPoint;
class PlotLineHandle;

class SCOPYGUI_EXPORT HandlesArea: public QWidget
{
	Q_OBJECT

public:
	HandlesArea(QWidget *parent);
	void installExtension(std::unique_ptr<HandlesAreaExtension> extension);

Q_SIGNALS:
	void sizeChanged(QSize);

protected:
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
	virtual void resizeMask(QSize) = 0;

	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

protected:
	QWidget *canvas;
	PlotLineHandle *selectedItem;
	QPoint hotspot;

	std::vector<std::unique_ptr<HandlesAreaExtension>> m_extensions;
};

class SCOPYGUI_EXPORT VertHandlesArea: public HandlesArea
{
	Q_OBJECT
public:
	VertHandlesArea(QWidget *parent = 0);

	int topPadding();
	void setTopPadding(int);
	int bottomPadding();
	void setBottomPadding(int);
	int largestChildHeight();
	void setLargestChildHeight(int);

protected:
	void resizeMask(QSize) Q_DECL_OVERRIDE;

private:
	int top_padding;
	int bottom_padding;
	int largest_child_height;
};

class SCOPYGUI_EXPORT HorizHandlesArea: public HandlesArea
{
	Q_OBJECT
public:
	HorizHandlesArea(QWidget *parent = 0);

	int leftPadding();
	void setLeftPadding(int);
	int rightPadding();
	void setRightPadding(int);
	int largestChildWidth();
	void setLargestChildWidth(int);

protected:
	void resizeMask(QSize) Q_DECL_OVERRIDE;

private:
	int left_padding;
	int right_padding;
	int largest_child_width;
};

class SCOPYGUI_EXPORT GateHandlesArea: public HorizHandlesArea
{
public:
	GateHandlesArea(QWidget *parent = 0);
protected:
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
};

#endif // HANDLES_AREA_H
