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
 */

#include "docking/docksettings.h"

#ifdef USE_KDDOCKWIDGETS
#include "style.h"

using namespace scopy;

NoCloseTitleBar::NoCloseTitleBar(KDDockWidgets::Core::TitleBar *controller, KDDockWidgets::Core::View *parent)
	: KDDockWidgets::QtWidgets::TitleBar(controller, parent)
	, m_controller(controller)
{
	Style::setBackgroundColor(this, json::theme::background_subtle, true);
}

void NoCloseTitleBar::init()
{
	m_controller->setHideDisabledButtons(KDDockWidgets::TitleBarButtonType::Close);
	KDDockWidgets::QtWidgets::TitleBar::init();
}

TitleBarFactory::TitleBarFactory()
	: KDDockWidgets::QtWidgets::ViewFactory()
{
	setObjectName("TitleBarFactory");
}

KDDockWidgets::Core::View *TitleBarFactory::createTitleBar(KDDockWidgets::Core::TitleBar *controller,
							   KDDockWidgets::Core::View *parent) const
{
	return new NoCloseTitleBar(controller, parent);
}
#endif // USE_KDDOCKWIDGETS
