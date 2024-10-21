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

#ifndef TINTEDOVERLAY_H
#define TINTEDOVERLAY_H

#include "scopy-gui_export.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QWidget>

namespace scopy::gui {
class SCOPY_GUI_EXPORT TintedOverlay : public QWidget
{
	Q_OBJECT
public:
	explicit TintedOverlay(QWidget *parent = nullptr, QColor color = QColor(0, 0, 0, 220));
	~TintedOverlay();

	const QList<QWidget *> &getHoles() const;
	void setHoles(const QList<QWidget *> &newHoles);
	void clearHoles();

protected:
	void paintEvent(QPaintEvent *ev) override;
	void mousePressEvent(QMouseEvent *ev) override;

private:
	QWidget *parent;
	QList<QWidget *> holes;
	QColor color;
};
} // namespace scopy::gui

#endif // TINTEDOVERLAY_H
