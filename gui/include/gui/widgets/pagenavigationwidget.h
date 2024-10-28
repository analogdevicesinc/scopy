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

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "scopy-gui_export.h"

#include <QPushButton>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT PageNavigationWidget : public QWidget
{
	Q_OBJECT
public:
	PageNavigationWidget(bool hasHome = false, bool hasOpen = false, QWidget *parent = nullptr);
	~PageNavigationWidget();

	QPushButton *getHomeBtn();
	QPushButton *getBackwardBtn();
	QPushButton *getForwardBtn();
	QPushButton *getOpenBtn();
	void setHomeBtnVisible(bool en);
	void setOpenBtnVisible(bool en);

private:
	QPushButton *homeButton;
	QPushButton *backwardButton;
	QPushButton *forwardButton;
	QPushButton *openButton;

	void initUI(QWidget *parent);
};
} // namespace scopy
#endif // NAVIGATIONWIDGET_H
