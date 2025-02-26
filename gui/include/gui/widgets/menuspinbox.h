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

#ifndef MENUSPINBOX_H
#define MENUSPINBOX_H

#include "plot_utils.hpp"
#include "utils.h"
#include "mousewheelwidgetguard.h"
#include <cmath>
#include <scopy-gui_export.h>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT IncrementStrategy
{
public:
	virtual ~IncrementStrategy(){};
	virtual double increment(double val) = 0;
	virtual double decrement(double val) = 0;
	virtual void setScale(double scale) = 0;
};

class SCOPY_GUI_EXPORT IncrementStrategy125 : public IncrementStrategy
{
public:
	NumberSeries m_steps;

	IncrementStrategy125()
		: m_steps(1e-9, 1e9, 10){};
	~IncrementStrategy125(){};
	virtual double increment(double val) override { return m_steps.getNumberAfter(val); }
	virtual double decrement(double val) override { return m_steps.getNumberBefore(val); }

	double m_scale;
	void setScale(double scale) override { m_scale = scale; }
};

class SCOPY_GUI_EXPORT IncrementStrategyPower2 : public IncrementStrategy
{
public:
	QList<double> m_steps;
	IncrementStrategyPower2()
	{
		for(int i = 30; i >= 0; i--) {
			m_steps.append(-(1 << i));
		}
		for(int i = 0; i < 31; i++) {
			m_steps.append(1 << i);
		}
	};
	~IncrementStrategyPower2(){};
	virtual double increment(double val) override
	{
		int i = 0;
		val = val + 1;
		while(val > m_steps[i]) {
			i++;
		}
		return m_steps[i];
	}
	virtual double decrement(double val) override
	{
		int i = m_steps.count() - 1;
		val = val - 1;
		while(val < m_steps[i]) {
			i--;
		}
		return m_steps[i];
	}
	double m_scale;

	void setScale(double scale) override { m_scale = scale; }
};
class SCOPY_GUI_EXPORT IncrementStrategyFixed : public IncrementStrategy
{
public:
	IncrementStrategyFixed(double k = 1)
	{
		m_k = k;
		m_scale = 1;
	};
	~IncrementStrategyFixed(){};
	virtual double increment(double val) override
	{
		val = val + m_k * m_scale;
		return val;
	}
	virtual double decrement(double val) override
	{
		val = val - m_k * m_scale;
		return val;
	}
	void setK(double val) { m_k = val; }
	double k() { return m_k; }

private:
	double m_k;
	double m_scale;

	void setScale(double scale) override { m_scale = scale; }
};

class SCOPY_GUI_EXPORT UnitPrefix
{
public:
	QString prefix;
	double scale;
	// enum type - metric, hour, logarithmic, etc
};

class SCOPY_GUI_EXPORT MenuSpinbox : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER

public:
	typedef enum
	{
		IS_POW2,
		IS_125,
		IS_FIXED

	} IncrementMode;

	MenuSpinbox(QString name, double val, QString unit, double min, double max, bool vertical = 0, bool left = 0,
		    bool large_widget = true, QWidget *parent = nullptr);
	~MenuSpinbox();

	double value() const;
	QString unit() const;
	IncrementStrategy *incrementStrategy() const;
	QString name() const;

	void setScaleRange(double min, double max);

public Q_SLOTS:
	void setName(const QString &newName);
	void setUnit(const QString &newUnit);
	void setMinValue(double);
	void setMaxValue(double);
	void setValueForce(double newValue, bool force = true);
	void setValueString(QString s);
	void setValue(double newValue);
	void setIncrementMode(IncrementMode is);
	void setScalingEnabled(bool en);

Q_SIGNALS:
	void nameChanged(QString);
	void valueChanged(double);
	void unitChanged(QString);

private Q_SLOTS:
	void userInput(QString s);
	void populateWidgets();

private:
	int findLastDigit(QString str);
	void layoutVertically(bool left);
	void layoutHorizontally(bool left);
	double clamp(double val, double min, double max);

	QLabel *m_label;
	QLineEdit *m_edit;
	QComboBox *m_scaleCb;
	QPushButton *m_plus;
	QPushButton *m_minus;
	MouseWheelWidgetGuard *m_mouseWheelGuard;

	IncrementStrategy *m_incrementStrategy;
	IncrementMode m_im;

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)

	QString m_name;
	double m_value, m_min, m_max;
	double m_scaleMin, m_scaleMax;
	QString m_unit;
	bool m_large_widget;

	QList<UnitPrefix> m_scales;
	// QMap<QString, double> m_scaleMap;
	double getScaleForPrefix(QString prefix, Qt::CaseSensitivity s = Qt::CaseSensitive);
	bool m_scalingEnabled;
};
} // namespace gui
} // namespace scopy
#endif // MENUSPINBOX_H
