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

#ifndef TITLESPINBOX_H
#define TITLESPINBOX_H

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT TitleSpinBox : public QWidget
{
	Q_OBJECT

public:
	explicit TitleSpinBox(QString title, bool isCompact = false, QWidget *parent = nullptr);
	~TitleSpinBox();

	void setTitle(QString title);
	QPushButton *getSpinBoxUpButton();
	QPushButton *getSpinBoxDownButton();
	QLineEdit *getLineEdit();

	double step() const;
	void setStep(double newStep);

	double max() const;
	void setMax(double newMax);

	double min() const;
	void setMin(double newMin);

	void setValue(double newValue);
	void setSpinButtonsDisabled(bool isDisabled);

private:
	/**
	 * @brief truncValue This function is needed because the QString::number function that
	 * would be normally used to parse a QString to a number does not work well with large
	 * numbers that end with multiple zeroes. The QString returned will be in scientific
	 * notation. The alternative is that we can set a number of fixed decimal points, but
	 * it would look awkward so this function converts it to a QString and chops the
	 * trailing zeroes.
	 * @param value
	 * @return
	 */
	static QString truncValue(double value);
	void connectSignalsAndSlots();

	QPushButton *m_spinBoxUpButton;
	QPushButton *m_spinBoxDownButton;
	QLabel *m_titleLabel;

	// This lineedit will act like a spinbox because the QSpinBox
	// is more restrictive than we would like :/
	QLineEdit *m_lineedit;
	double m_min;
	double m_max;
	double m_step;
};
} // namespace scopy
#endif // TITLESPINBOX_H
