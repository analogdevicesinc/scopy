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

#ifndef LAZYLOADWIDGET_H
#define LAZYLOADWIDGET_H

#include <QWidget>
#include <QShowEvent>

/**
 * Used to instantly perform the lazy load in a class that contains the QWIDGET_LAZY_INIT macro.
 */
#define LAZY_LOAD(func)                                                                                                \
	do {                                                                                                           \
		func();                                                                                                \
		m_lazy_load_initialized = true;                                                                        \
	} while(0)

/**
 * Used in a class header to make the class use the lazy load mechanism. The parameter is the name of function
 * that will be called as part of the initializaiton process. CAUTION: The function signature should not have
 * any mandatory parameters.
 */
#define QWIDGET_LAZY_INIT(func)                                                                                        \
protected:                                                                                                             \
	void showEvent(QShowEvent *event) override                                                                     \
	{                                                                                                              \
		if(!m_lazy_load_initialized) {                                                                         \
			LAZY_LOAD(func);                                                                               \
		}                                                                                                      \
		QWidget::showEvent(event);                                                                             \
	}                                                                                                              \
	bool m_lazy_load_initialized = false;

#endif // LAZYLOADWIDGET_H
