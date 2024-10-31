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

#ifndef SCOPY_ERRORBOX_H
#define SCOPY_ERRORBOX_H

#include <QFrame>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ErrorBox : public QFrame
{
	Q_OBJECT
public:
	typedef enum
	{
		Green,
		Yellow,
		Red,
		Transparent,
	} AvailableColors;

	explicit ErrorBox(QWidget *parent = nullptr);

public Q_SLOTS:
	void changeColor(AvailableColors color);
};
} // namespace scopy

#endif // SCOPY_ERRORBOX_H
