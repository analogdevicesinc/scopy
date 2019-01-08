/*
 * Copyright 2018 Analog Devices, Inc.
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

#include "startstoprangewidget.h"
#include "ui_startstoprangewidget.h"

using namespace adiscope;

StartStopRangeWidget::StartStopRangeWidget(double min, double max, double minSpan,
		bool hasProgressWidget,
		QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StartStopRangeWidget),
	minValue(min),
	maxValue(max),
	minSpan(minSpan)
{
	ui->setupUi(this);

	_createSpinButtons(hasProgressWidget);
	_setupSignalsAndSlots();
}

StartStopRangeWidget::~StartStopRangeWidget()
{
	delete ui;
}

double StartStopRangeWidget::getStartValue() const
{
	return start_freq->value();
}

void StartStopRangeWidget::setStartValue(double value)
{
	start_freq->setValue(value);
}

double StartStopRangeWidget::getStopValue() const
{
	return stop_freq->value();
}

void StartStopRangeWidget::setStopValue(double value)
{
	stop_freq->setValue(value);
}

double StartStopRangeWidget::getCenterValue() const
{
	return center_freq->value();
}

void StartStopRangeWidget::setMinimumSpanValue(double value)
{
	minSpan = value;
	span_freq->setMinValue(value);
}

void StartStopRangeWidget::_onStartStopFrequencyChanged()
{
	double start = start_freq->value();
	double stop = stop_freq->value();

	start_freq->setMaxValue(stop - 1);
	stop_freq->setMinValue(start + 1);

	double span = stop - start;
	double center = start + (span / 2);

	span_freq->silentSetValue(span);
	center_freq->silentSetValue(center);

	Q_EMIT rangeChanged(start, stop);
}

void StartStopRangeWidget::_onCenterSpanFrequencyChanged()
{
	double span = span_freq->value();
	double center = center_freq->value();
	double start = center - (span / 2);
	double stop = center + (span / 2);

	if (QObject::sender() == center_freq) {
		// Center value was changed by the user, so we
		// check if the span value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			span = (center - start) * 2;
			stop = center + (span / 2);
		} else if (stop > stop_freq->maxValue()) {
			stop = stop_freq->maxValue();
			span = (stop - center) * 2;
			start = center - (span / 2);
		}

		span_freq->silentSetValue(span);
	} else {
		// Span value was changed by the user, so we
		// check if the center value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			center = start + (span / 2);
			stop = center + (span / 2);
		} else if (stop > stop_freq->maxValue()) {
			stop = stop_freq->maxValue();
			center = stop - (span / 2);
			start = center - (span / 2);
		}

		center_freq->silentSetValue(center);
	}

	start_freq->silentSetMaxValue(stop - 1);
	stop_freq->silentSetMinValue(start + 1);

	start_freq->silentSetValue(start);
	stop_freq->silentSetValue(stop);

	Q_EMIT rangeChanged(start, stop);
}

void StartStopRangeWidget::_createSpinButtons(bool hasProgressWidget)
{
	// Create spin buttons
	start_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Start", minValue, maxValue,
	hasProgressWidget, false, this,
	{1, 2.5, 5, 7.5});
	stop_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Stop", minValue, maxValue,
	hasProgressWidget, false, this,
	{1, 2.5, 5, 7.5});
	center_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Center", minValue, maxValue - 1,
	hasProgressWidget, false, this,
	{1, 2.5, 5, 7.5});
	span_freq = new ScaleSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	},"Span", minValue, maxValue - 1,
	hasProgressWidget, false, this,
	{1, 2.5, 5, 7.5});

	// Set limits and initial values
	start_freq->setValue(1000);
	stop_freq->setValue(50000);
	start_freq->enableNumberSeriesRebuild(false);
	stop_freq->enableNumberSeriesRebuild(false);
	center_freq->enableNumberSeriesRebuild(false);
	span_freq->enableNumberSeriesRebuild(false);

	// Add to layout
	ui->startValueLayout->addWidget(start_freq);
	ui->stopValueLayout->addWidget(stop_freq);
	ui->centerValueLayout->addWidget(center_freq);
	ui->spanValueLayout->addWidget(span_freq);
}

void StartStopRangeWidget::_setupSignalsAndSlots()
{
	connect(start_freq, &ScaleSpinButton::valueChanged,
		this, &StartStopRangeWidget::_onStartStopFrequencyChanged);
	connect(stop_freq, &ScaleSpinButton::valueChanged,
		this, &StartStopRangeWidget::_onStartStopFrequencyChanged);
	connect(center_freq, &ScaleSpinButton::valueChanged,
		this, &StartStopRangeWidget::_onCenterSpanFrequencyChanged);
	connect(span_freq, &ScaleSpinButton::valueChanged,
		this, &StartStopRangeWidget::_onCenterSpanFrequencyChanged);
}
