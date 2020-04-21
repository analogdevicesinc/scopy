/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SCOPY_MATH_HPP
#define SCOPY_MATH_HPP

#include "ui_math.h"

#include <QAction>
#include <QWidget>

namespace adiscope {
class Math : public QWidget {
	Q_OBJECT

public:
	explicit Math(QWidget *parent = nullptr, unsigned int num_inputs = 1);

public Q_SLOTS:
	void setFunction(const QString &function);

Q_SIGNALS:
	void functionValid(const QString &function);
	void stateReseted();

private Q_SLOTS:
	void handleButton();
	void handleMenuButton(QAction *action);
	void handleMenuButtonT(QAction *action);
	void validateFunction();
	void resetState();
	void delLastChar();

private:
	Ui::Math ui;
	unsigned int num_inputs;
};
} // namespace adiscope

#endif /* SCOPY_MATH_HPP */
