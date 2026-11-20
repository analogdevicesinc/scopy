/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include "scopy-gui_export.h"

#include <QLayout>
#include <QList>

namespace scopy {

// Lays its items left to right and wraps them onto new rows, so a row of labels follows the
// width it is given instead of forcing it. Reports its height for a width, so it can sit in a
// vertically scrolling area.
class SCOPY_GUI_EXPORT FlowLayout : public QLayout
{
	Q_OBJECT
public:
	explicit FlowLayout(QWidget *parent = nullptr, int hSpacing = 12, int vSpacing = 6);
	~FlowLayout() override;

	void addItem(QLayoutItem *item) override;
	int count() const override;
	QLayoutItem *itemAt(int index) const override;
	QLayoutItem *takeAt(int index) override;

	Qt::Orientations expandingDirections() const override;
	bool hasHeightForWidth() const override;
	int heightForWidth(int width) const override;
	QSize minimumSize() const override;
	QSize sizeHint() const override;
	void setGeometry(const QRect &rect) override;

private:
	int doLayout(const QRect &rect, bool apply) const;

	QList<QLayoutItem *> m_items;
	int m_hSpacing;
	int m_vSpacing;
};

} // namespace scopy
