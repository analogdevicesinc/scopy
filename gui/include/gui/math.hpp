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

#ifndef SCOPY_MATH_HPP
#define SCOPY_MATH_HPP

#include <QAction>
#include <QWidget>
#include "scopygui_export.h"

namespace Ui {
	class Math;
}

namespace scopy {
	class SCOPYGUI_EXPORT Math : public QWidget
	{
		Q_OBJECT

	public:
		explicit Math(QWidget *parent = nullptr,
				unsigned int num_inputs = 1);

	public Q_SLOTS:
		void setFunction(const QString& function);

	Q_SIGNALS:
		void functionValid(const QString& function);
		void stateReseted();

	private Q_SLOTS:
		void handleButton();
		void handleMenuButton(QAction *action);
		void handleMenuButtonT(QAction *action);
		void validateFunction();
		void resetState();
		void delLastChar();

	private:
		Ui::Math *ui;
		unsigned int num_inputs;
	};
}

#endif /* SCOPY_MATH_HPP */
