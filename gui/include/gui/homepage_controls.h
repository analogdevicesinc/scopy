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

#ifndef HOMEPAGE_CONTROLS_H
#define HOMEPAGE_CONTROLS_H

#include "scopy-gui_export.h"

#include <QString>
#include <QWidget>

#include <widgets/hoverwidget.h>
#include <widgets/pagenavigationwidget.h>

namespace scopy {
class SCOPY_GUI_EXPORT HomepageControls : public HoverWidget
{
	Q_OBJECT

public:
	explicit HomepageControls(QWidget *parent = 0);
	~HomepageControls();

public Q_SLOTS:
	void enableLeft(bool);
	void enableRight(bool);

Q_SIGNALS:
	void goLeft();
	void goRight();
	void openFile();

private:
	PageNavigationWidget *controls;
	void connectSignals();
};
} // namespace scopy
#endif // HOMEPAGE_CONTROLS_H
