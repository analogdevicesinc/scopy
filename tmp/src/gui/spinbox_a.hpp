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

#ifndef SPIN_BOX_A_H
#define SPIN_BOX_A_H

#include <QStringList>
#include <QWidget>
#include <vector>
#include <QSettings>

#include "plot_utils.hpp"
#include "apiObject.hpp"

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
class SpinBoxA_API;

class SpinBoxA : public QWidget
{
	friend class SpinBoxA_API;

	Q_OBJECT

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

	bool isStepDown() const;

	void triggerCircleRedraw();

	QString getName() const;
	void setName(const QString& name);

	void setDisplayScale(double value);

	static unsigned int current_id;

	void updateCompletionCircle(double value);

	void silentSetValue(double value);
	void silentSetMinValue(double value);
	void silentSetMaxValue(double value);

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
	SpinBoxA_API *m_sba_api;
	unsigned int m_id;
	bool m_is_step_down;

private:
	QSettings *m_settings;
};

class SpinBoxA_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(double min_value READ minValue WRITE setMinValue STORED false)
	Q_PROPERTY(double max_value READ maxValue WRITE setMaxValue STORED false)
	Q_PROPERTY(double value READ value WRITE setValue STORED false)
	Q_PROPERTY(bool fine_mode READ isInFineMode WRITE setFineMode)
	Q_PROPERTY(bool invert_circle READ isCircleInverted WRITE invertCircle STORED false)
	Q_PROPERTY(bool show_progress READ progressShown WRITE showProgress STORED false)
	Q_PROPERTY(QString name READ getName WRITE setName STORED false)

public:
	explicit SpinBoxA_API(SpinBoxA *sba);
	~SpinBoxA_API();

	double value() const;
	void setValue(double value);

	double minValue() const;
	void setMinValue(double value);

	double maxValue() const;
	void setMaxValue(double value);

	int decimalCount() const;
	void setDecimalCount(int count);

	bool isInFineMode();
	void setFineMode(bool enabled);

	bool isCircleInverted() const;
	void invertCircle(bool invert);

	bool progressShown() const;
	void showProgress(bool show);

	bool fineModeAvailable();
	void setFineModeAvailable(bool available);

	QString getName() const;
	void setName(const QString& name);

private:
	SpinBoxA *sba;
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
	int getIntegerDivider();
	void setIntegerDivider(int integer = 0);

public Q_SLOTS:
	void stepUp();
	void stepDown();

	void setMinValue(double);
	void setMaxValue(double);
	void setValue(double);

	void enableNumberSeriesRebuild(bool enable);

protected:
	adiscope::NumberSeries m_steps;
	double m_fine_increment;
	bool m_numberSeriesRebuild;
	int integer_divider;
	int integer_step;
};

class PositionSpinButton: public SpinBoxA
{
	Q_OBJECT
public:
	explicit PositionSpinButton(QWidget *parent = nullptr);
	explicit PositionSpinButton(std::vector<std::pair<QString, double> >units,
	                            const QString& name ="",
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
	explicit PhaseSpinButton(std::vector<std::pair<QString, double> >units,
	                         const QString& name ="",
	                         double min_value = 0.0, double max_value = 360.0,
	                         bool hasProgressWidget = true,
	                         bool invertCircle = false, QWidget *parent = 0);

	void setStep(double);

public Q_SLOTS:
	void stepUp();
	void stepDown();

	void setValue(double value);
	void setComboboxIndex(int index);
	void onComboboxIndexChanged(int index);
	void updatePhaseAfterFrequenceChanged(double val);
	double computeSecondsTransformation(double scale, int index, double value);
	void setInSeconds(bool val);
	bool inSeconds();
	void setSecondsValue(double val);
	double secondsValue();
	void setFrequency(double val);
	double frequency();
	double value();
	double changeValueFromDegreesToSeconds(double val);
	int indexValue();

protected:
	std::vector<std::vector<double>> m_unit;
	double m_step;
	double m_fine_increment;
	double m_frequency;
	double m_secondsValue;
	bool m_inSeconds;
	double m_scale;
	int m_indexValue;

};

}

#endif // SPIN_BOX_A_H
