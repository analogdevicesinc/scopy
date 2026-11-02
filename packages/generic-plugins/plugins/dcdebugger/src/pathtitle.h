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

#ifndef PATHTITLE_H
#define PATHTITLE_H

#include <QList>
#include <QPair>
#include <QPointer>
#include <QWidget>

namespace scopy::dcdebugger {

// Breadcrumb of a component path: one button per segment, a click selects that component.
class PathTitle : public QWidget
{
	Q_OBJECT
public:
	using Segment = QPair<QString, QObject *>;

	explicit PathTitle(const QList<Segment> &segments, QWidget *parent = nullptr);

Q_SIGNALS:
	void pathSelected(QObject *node);
};

} // namespace scopy::dcdebugger

#endif // PATHTITLE_H
