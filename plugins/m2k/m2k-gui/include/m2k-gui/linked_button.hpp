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

#ifndef LINKEDBUTTON_HPP
#define LINKEDBUTTON_HPP

#include "scopy-m2k-gui_export.h"
#include <QPushButton>

namespace scopy {

class SCOPY_M2K_GUI_EXPORT LinkedButton : public QPushButton
{
	Q_OBJECT

public:
	explicit LinkedButton(QWidget *parent = Q_NULLPTR);

	void setUrl(QString url);

private:
	QString url;

public Q_SLOTS:
	void openUrl();
};
} // namespace scopy

#endif // LINKEDBUTTON_HPP
