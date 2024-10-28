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

#ifndef HOVERWIDGET_H
#define HOVERWIDGET_H

#include "scopy-gui_export.h"
#include "utils.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QWidget>

namespace scopy {
enum HoverPosition
{
	HP_LEFT,
	HP_TOPLEFT,
	HP_TOP,
	HP_TOPRIGHT,
	HP_RIGHT,
	HP_BOTTOMRIGHT,
	HP_BOTTOM,
	HP_BOTTOMLEFT,
	HP_CENTER,
};

class SCOPY_GUI_EXPORT HoverWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	HoverWidget(QWidget *content = nullptr, QWidget *anchor = nullptr, QWidget *parent = nullptr);
	~HoverWidget();

	bool eventFilter(QObject *watched, QEvent *event) override;
	QPoint getAnchorOffset();
	void setAnchorOffset(QPoint);
	HoverPosition getAnchorPos();
	void setAnchorPos(HoverPosition);
	HoverPosition getContentPos();
	void setContentPos(HoverPosition);

public:
	void updatePos();
	void moveToRelativePos();
	void moveToAnchor();
	void setContent(QWidget *widget);
	void setAnchor(QWidget *anchor);
	void setParent(QWidget *parent);
	void setDraggable(bool draggable);
	void setRelative(bool relative);
	void setRelativeOffset(QPointF offset);

private:
	QHBoxLayout *m_lay;
	QWidget *m_parent;
	QWidget *m_anchor;
	QWidget *m_content;
	QPoint m_anchorOffset;
	HoverPosition m_anchorPos;
	HoverPosition m_contentPos;
	bool m_draggable;
	bool is_dragging;
	QPoint *mouse_pos;
	bool m_relative;
	QPointF *m_relativeOffset;

private:
	void updateRelativeOffset();

	// QWidget interface
protected:
	void showEvent(QShowEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
};
} // namespace scopy
#endif // HOVERWIDGET_H
