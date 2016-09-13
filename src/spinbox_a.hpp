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

#ifndef SPIN_BOX_A_H
#define SPIN_BOX_A_H

#include <QWidget>
#include <vector>

#include "plot_utils.hpp"

class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class QFrame;
class QComboBox;
class QRegExpValidator;
class CompletionCircle;

namespace Ui {
	class SpinBoxA;
}

/*
 * SpinBoxA - Is a composite widget which contains the following:
 *   - a QLineEdit (displays the value of SpinBoxA)
 *   - two QPushButton (to replace the QSpinBox buttons)
 *   - a QLabel (name of SpinBoxA)
 *   - a QFrame (line between QSpinBox and QComboBox)
 *   - a QComboBox (scale and measurement unit of SpinBoxA)
 *   - a CompletionCircle: graphically displays the position of the current
 *       value of the SpinBoxA in the full range of values
 *
 * SpinBoxA = Spinbox of type A. If other composite widgets around
 *            a QSpinBox will be created, they can be called B,C,..
 */
class SpinBoxA : public QWidget
{
	Q_OBJECT
public:
	explicit SpinBoxA(std::vector<std::pair<QString, double> >units,
			const QString &name = "",
			double min_value = 0.0, double max_value = 0.0,
			bool hasProgressWidget = true,
			bool invertCircle = false, QWidget *parent = 0);
	~SpinBoxA();

	QPushButton * upButton();
	QPushButton * downButton();
	QLabel * nameLabel();
	QLineEdit * lineEdit();
	QFrame * line();
	QComboBox *comboBox();

	double value();

	double minValue();
	virtual void setMinValue(double);

	double maxValue();
	virtual void setMaxValue(double);

	bool isInFineMode();

public Q_SLOTS:
	void setValue(double);
	void setFineMode(bool);

	virtual void stepUp() = 0;
	virtual void stepDown() = 0;

Q_SIGNALS:
	void valueChanged(double);

protected Q_SLOTS:
	void onUpButtonPressed();
	void onDownButtonPressed();
	void onComboboxIndexChanged(int);
	void onLineEditTextEdited();

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);
	double findUnitOfValue(double val,int *posInUnitsList = NULL);

protected:
	Ui::SpinBoxA *ui;
	CompletionCircle *m_SBA_CompCircle;

	double m_value;
	double m_min_value;
	double m_max_value;
	std::vector<std::pair<QString, double> > m_units;
	bool m_fine_mode;
	QRegExpValidator *m_validator;
};

class ScaleSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit ScaleSpinButton(std::vector<std::pair<QString, double> >units,
			const QString &name = "",
			double min_value = 0.0, double max_value = 0.0,
			bool hasProgressWidget = true,
			bool invertCircle = false, QWidget *parent = 0);

	void setMinValue(double);
	void setMaxValue(double);

public Q_SLOTS:
	void stepUp();
	void stepDown();

protected:
	adiscope::NumberSeries m_steps;
	double m_fine_increment;
};

class PositionSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit PositionSpinButton(std::vector<std::pair<QString, double> >units,
			const QString &name = "",
			double min_value = 0.0, double max_value = 0.0,
			bool hasProgressWidget = true,
			bool invertCircle = false, QWidget *parent = 0);

	double step();
	void setStep(double);

public Q_SLOTS:
	void stepUp();
	void stepDown();

protected:
	double m_step;
};

#endif // SPIN_BOX_A_H
