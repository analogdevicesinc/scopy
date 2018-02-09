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

#include <QStringList>
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

namespace Ui {
class SpinBoxA;
}

namespace adiscope {
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

	Q_PROPERTY(QStringList units MEMBER m_units_list WRITE setUnits);
	Q_PROPERTY(double min_value READ minValue WRITE setMinValue);
	Q_PROPERTY(double max_value READ maxValue WRITE setMaxValue);
	Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged);
	Q_PROPERTY(bool fine_mode READ isInFineMode WRITE setFineMode);
	Q_PROPERTY(bool invert_circle READ isCircleInverted WRITE invertCircle);
	Q_PROPERTY(bool show_progress READ progressShown WRITE showProgress);
	Q_PROPERTY(QString name READ getName WRITE setName);

public:
	explicit SpinBoxA(QWidget *parent = nullptr);
	explicit SpinBoxA(std::vector<std::pair<QString, double> >units,
	                  const QString& name = "",
	                  double min_value = 0.0, double max_value = 0.0,
	                  bool hasProgressWidget = true,
	                  bool invertCircle = false, QWidget *parent = 0);
	~SpinBoxA();

	QPushButton *upButton();
	QPushButton *downButton();
	QLabel *nameLabel();
	QLineEdit *lineEdit();
	QFrame *line();
	QComboBox *comboBox();

	double value();

	double minValue();
	double maxValue();

	int decimalCount() const;
	void setDecimalCount(int);

	bool isInFineMode();

	bool isCircleInverted() const;
	void invertCircle(bool invert);

	bool progressShown() const;
	void showProgress(bool show);

    bool fineModeAvailable();
    void setFineModeAvailable(bool);


	QString getName() const;
	void setName(const QString& name);

	void setDisplayScale(double value);

public Q_SLOTS:
	virtual void setValue(double);
	void setFineMode(bool);

	virtual void setMinValue(double);
	virtual void setMaxValue(double);

	virtual void stepUp() = 0;
	virtual void stepDown() = 0;

Q_SIGNALS:
	void valueChanged(double);

protected Q_SLOTS:
	void onUpButtonPressed();
	void onDownButtonPressed();
	virtual void onComboboxIndexChanged(int);
	void onLineEditTextEdited();

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);
	double findUnitOfValue(double val,int *posInUnitsList = NULL);
	void setUnits(const QStringList& map);
	bool isUnitMatched(const QString& unit, double value);

protected:
	Ui::SpinBoxA *ui;
	QStringList m_units_list;

	double m_value;
	double m_min_value;
	double m_max_value;
	int m_decimal_count;
	std::vector<std::pair<QString, double> > m_units;
	QRegExpValidator *m_validator;
	double m_displayScale;
};

class ScaleSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit ScaleSpinButton(QWidget *parent = nullptr);
	explicit ScaleSpinButton(std::vector<std::pair<QString, double> >units,
	                         const QString& name = "",
	                         double min_value = 0.0, double max_value = 0.0,
	                         bool hasProgressWidget = true,
                 bool invertCircle = false, QWidget *parent = 0, std::vector<double> steps = {1,2,5});

public Q_SLOTS:
	void stepUp();
	void stepDown();

	void setMinValue(double);
	void setMaxValue(double);

protected:
	adiscope::NumberSeries m_steps;
	double m_fine_increment;
};

class PositionSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit PositionSpinButton(QWidget *parent = nullptr);
	explicit PositionSpinButton(std::vector<std::pair<QString, double> >units,
	                            const QString& name = "",
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


class PhaseSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit PhaseSpinButton(QWidget *parent = nullptr);

	double step();
	void setStep(double);

public Q_SLOTS:
	void stepUp();
	void stepDown();

	void setValue(double value);
	void onComboboxIndexChanged(int index);

protected:
	std::vector<std::vector<double>> m_unit;
	double m_step;
	double m_fine_increment;

};

}

#endif // SPIN_BOX_A_H
