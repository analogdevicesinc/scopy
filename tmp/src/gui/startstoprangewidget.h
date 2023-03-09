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

#ifndef STARTSTOPRANGEWIDGET_H
#define STARTSTOPRANGEWIDGET_H

#include <QWidget>

#include "spinbox_a.hpp"

namespace Ui {
class StartStopRangeWidget;
}

namespace adiscope {
class StartStopRangeWidget : public QWidget
{
	Q_OBJECT

public:
	explicit StartStopRangeWidget(double min = 1.0, double max = 5e07,
				      double minSpan = 1000,
				      bool hasProgressWidget = false,
				      QWidget *parent = nullptr);
	~StartStopRangeWidget();

	double getStartValue() const;
	void setStartValue(double value);

	double getStopValue() const;
	void setStopValue(double value);

	double getCenterValue() const;

	void insertWidgetIntoLayout(QWidget *widget, int row, int column);

	void setMinimumValue(double value);

public Q_SLOTS:
	void setMinimumSpanValue(double value);

Q_SIGNALS:
	void rangeChanged(double, double);

private Q_SLOTS:
	void _onStartStopFrequencyChanged();
	void _onCenterSpanFrequencyChanged();

private:
	void _createSpinButtons(bool hasProgressWidget = false);
	void _setupSignalsAndSlots();

private:
	Ui::StartStopRangeWidget *ui;
	ScaleSpinButton *start_freq;
	ScaleSpinButton *stop_freq;
	ScaleSpinButton *center_freq;
	ScaleSpinButton *span_freq;

	double minValue;
	double maxValue;
	double minSpan;
};
}

#endif // STARTSTOPRANGEWIDGET_H
