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

#ifndef DOCKSETTINGS_H
#define DOCKSETTINGS_H

#ifndef USE_KDDOCKWIDGETS
namespace scopy {
static void initDockWidgets() {}
} // namespace scopy
#else
#include "scopy-gui_export.h"

#include <kddockwidgets/KDDockWidgets.h>
#include <kddockwidgets/Config.h>

#include <kddockwidgets/core/TitleBar.h>
#include <kddockwidgets/core/View.h>
#include <kddockwidgets/core/Group.h>
#include <kddockwidgets/core/Group.h>

#include <kddockwidgets/qtwidgets/TitleBar.h>
#include <kddockwidgets/qtwidgets/ViewFactory.h>
#include <kddockwidgets/qtwidgets/Group.h>

namespace scopy {
// All of this just to hide a button
class SCOPY_GUI_EXPORT NoCloseTitleBar : public KDDockWidgets::QtWidgets::TitleBar
{
public:
	explicit NoCloseTitleBar(KDDockWidgets::Core::TitleBar *controller,
				 KDDockWidgets::Core::View *parent = nullptr);
	void init() override;

private:
	KDDockWidgets::Core::TitleBar *const m_controller;
};

class SCOPY_GUI_EXPORT TitleBarFactory : public KDDockWidgets::QtWidgets::ViewFactory
{
public:
	explicit TitleBarFactory();
	KDDockWidgets::Core::View *createTitleBar(KDDockWidgets::Core::TitleBar *controller,
						  KDDockWidgets::Core::View *parent) const override;
};

// Mark as static or inline to avoid ODR violation, this should only be used once in main.cpp anyway
static void initDockWidgets()
{
	KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
	KDDockWidgets::Config::self().setViewFactory(new TitleBarFactory());
}
} // namespace scopy

#endif // USE_KDDOCKWIDGETS
#endif // DOCKSETTINGS_H
