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

#include "ui_start_stop_range_widget.h"

#include <scopy/gui/start_stop_range_widget.hpp>

using namespace scopy::gui;

StartStopRangeWidget::StartStopRangeWidget(double min, double max, double minSpan, bool hasProgressWidget,
					   QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::StartStopRangeWidget)
	, m_minValue(min)
	, m_maxValue(max)
	, m_minSpan(minSpan)
{
	m_ui->setupUi(this);

	_createSpinButtons(hasProgressWidget);
	_setupSignalsAndSlots();
}

StartStopRangeWidget::~StartStopRangeWidget() { delete m_ui; }

double StartStopRangeWidget::getStartValue() const { return m_startFreq->value(); }

void StartStopRangeWidget::setStartValue(double value) { m_startFreq->setValue(value); }

double StartStopRangeWidget::getStopValue() const { return m_stopFreq->value(); }

void StartStopRangeWidget::setStopValue(double value) { m_stopFreq->setValue(value); }

double StartStopRangeWidget::getCenterValue() const { return m_centerFreq->value(); }

void StartStopRangeWidget::insertWidgetIntoLayout(QWidget* widget, int row, int column)
{
	m_ui->gridLayout->addWidget(widget, row, column);
}

void StartStopRangeWidget::setMinimumSpanValue(double value)
{
	m_minSpan = value;
	m_spanFreq->setMinValue(value);
}

void StartStopRangeWidget::_onStartStopFrequencyChanged()
{
	double start = m_startFreq->value();
	double stop = m_stopFreq->value();

	m_startFreq->setMaxValue(stop - 1);
	m_stopFreq->setMinValue(start + 1);

	double span = stop - start;
	double center = start + (span / 2);

	m_spanFreq->silentSetValue(span);
	m_centerFreq->silentSetValue(center);

	Q_EMIT rangeChanged(start, stop);
}

void StartStopRangeWidget::_onCenterSpanFrequencyChanged()
{
	double span = m_spanFreq->value();
	double center = m_centerFreq->value();
	double start = center - (span / 2);
	double stop = center + (span / 2);

	if (QObject::sender() == m_centerFreq) {
		// Center value was changed by the user, so we
		// check if the span value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			span = (center - start) * 2;
			stop = center + (span / 2);
		} else if (stop > m_stopFreq->maxValue()) {
			stop = m_stopFreq->maxValue();
			span = (stop - center) * 2;
			start = center - (span / 2);
		}

		m_spanFreq->silentSetValue(span);
	} else {
		// Span value was changed by the user, so we
		// check if the center value is valid, if not we
		// adjust it
		if (start < 0) {
			start = 1;
			center = start + (span / 2);
			stop = center + (span / 2);
		} else if (stop > m_stopFreq->maxValue()) {
			stop = m_stopFreq->maxValue();
			center = stop - (span / 2);
			start = center - (span / 2);
		}

		m_centerFreq->silentSetValue(center);
	}

	m_startFreq->silentSetMaxValue(stop - 1);
	m_stopFreq->silentSetMinValue(start + 1);

	m_startFreq->silentSetValue(start);
	m_stopFreq->silentSetValue(stop);

	Q_EMIT rangeChanged(start, stop);
}

void StartStopRangeWidget::setMinimumValue(double value)
{
	m_minValue = value;
	m_startFreq->setMinValue(value);
	m_stopFreq->setMinValue(value);
	m_centerFreq->setMinValue(value);
	m_spanFreq->setMinValue(value);
}

void StartStopRangeWidget::_createSpinButtons(bool hasProgressWidget)
{
	// Create spin buttons
	m_startFreq = new ScaleSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, tr("Start"), m_minValue,
					  m_maxValue, hasProgressWidget, false, this, {1, 2.5, 5, 7.5});
	m_stopFreq = new ScaleSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, tr("Stop"), m_minValue, m_maxValue,
					 hasProgressWidget, false, this, {1, 2.5, 5, 7.5});
	m_centerFreq = new ScaleSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, tr("Center"), m_minValue,
					   m_maxValue - 1, hasProgressWidget, false, this, {1, 2.5, 5, 7.5});
	m_spanFreq = new ScaleSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, tr("Span"), m_minValue,
					 m_maxValue - 1, hasProgressWidget, false, this, {1, 2.5, 5, 7.5});

	// Set limits and initial values
	m_startFreq->setValue(1000);
	m_stopFreq->setValue(50000);
	m_startFreq->enableNumberSeriesRebuild(false);
	m_stopFreq->enableNumberSeriesRebuild(false);
	m_centerFreq->enableNumberSeriesRebuild(false);
	m_spanFreq->enableNumberSeriesRebuild(false);

	// Add to layout
	m_ui->layoutStartValue->addWidget(m_startFreq);
	m_ui->layoutStopValue->addWidget(m_stopFreq);
	m_ui->layoutCenterValue->addWidget(m_centerFreq);
	m_ui->layoutSpanValue->addWidget(m_spanFreq);
}

void StartStopRangeWidget::_setupSignalsAndSlots()
{
	connect(m_startFreq, &ScaleSpinButton::valueChanged, this, &StartStopRangeWidget::_onStartStopFrequencyChanged);
	connect(m_stopFreq, &ScaleSpinButton::valueChanged, this, &StartStopRangeWidget::_onStartStopFrequencyChanged);
	connect(m_centerFreq, &ScaleSpinButton::valueChanged, this,
		&StartStopRangeWidget::_onCenterSpanFrequencyChanged);
	connect(m_spanFreq, &ScaleSpinButton::valueChanged, this, &StartStopRangeWidget::_onCenterSpanFrequencyChanged);
}
