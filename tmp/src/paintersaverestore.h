/*
 * Copyright (c) 2020 Analog Devices Inc.
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


#ifndef PAINTERSAVERESTORE_H
#define PAINTERSAVERESTORE_H

#include <QtGlobal>
#include <QPainter>

/*
* RAII class for saving and restoring a QPainter
*/
class PainterSaveRestore
{
public:
	PainterSaveRestore(QPainter *painter);
	~PainterSaveRestore();

private:
	QPainter *m_painter;
};

#endif // PAINTERSAVERESTORE_H
