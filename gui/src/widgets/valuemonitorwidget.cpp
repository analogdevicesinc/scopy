/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 */

#include "valuemonitorwidget.h"

#include "lcdNumber.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <style.h>

using namespace scopy;

ValueMonitorWidget::ValueMonitorWidget(const QString &name, const QString &unit, const QString &rowLabel,
				       unsigned precision, const QColor &color, bool peakHoldVisible, QWidget *parent)
	: QFrame(parent)
{
	QGridLayout *layout = new QGridLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	m_name = new QLabel(this);
	layout->addWidget(m_name, 0, 0, 1, 1, Qt::AlignLeft);

	m_value = new LcdNumber(this);
	m_value->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
	m_value->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_value->setMinimumSize(150, 50);
	layout->addWidget(m_value, 0, 1, 1, 3, Qt::AlignCenter);

	m_peakHold = new QWidget(this);
	QVBoxLayout *peakLayout = new QVBoxLayout(m_peakHold);
	peakLayout->setContentsMargins(0, 0, 0, 0);

	m_minLabel = new QLabel("min", m_peakHold);
	m_min = new LcdNumber(m_peakHold);
	m_min->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
	QHBoxLayout *minRow = new QHBoxLayout();
	minRow->setContentsMargins(0, 0, 0, 0);
	minRow->addWidget(m_minLabel);
	minRow->addWidget(m_min, 3);
	peakLayout->addLayout(minRow);

	m_maxLabel = new QLabel("max", m_peakHold);
	m_max = new LcdNumber(m_peakHold);
	m_max->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
	QHBoxLayout *maxRow = new QHBoxLayout();
	maxRow->setContentsMargins(0, 0, 0, 0);
	maxRow->addWidget(m_maxLabel);
	maxRow->addWidget(m_max, 3);
	peakLayout->addLayout(maxRow);

	layout->addWidget(m_peakHold, 0, 4, 1, 1, Qt::AlignRight);

	m_unit = new QLabel(this);
	m_unit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout->addWidget(m_unit, 0, 5);

	m_rowLabel = new QLabel(this);
	layout->addWidget(m_rowLabel);

	setMonitorName(name);
	setUnit(unit);
	setRowLabel(rowLabel);
	setPrecision(precision);
	setColor(color);
	setPeakHoldVisible(peakHoldVisible);
}

ValueMonitorWidget::ValueMonitorWidget(QWidget *parent)
	: ValueMonitorWidget(QString(), QString(), QString(), 3, QColor(), true, parent)
{}

void ValueMonitorWidget::setMonitorName(const QString &name)
{
	m_name->setText(name);
	m_name->setVisible(!name.isEmpty());
}

void ValueMonitorWidget::setUnit(const QString &unit) { m_unit->setText(unit); }

void ValueMonitorWidget::setRowLabel(const QString &label)
{
	m_rowLabel->setText(label);
	m_rowLabel->setVisible(!label.isEmpty());
}

void ValueMonitorWidget::setPrecision(unsigned precision)
{
	for(LcdNumber *lcd : {m_value, m_min, m_max}) {
		lcd->setPrecision(precision);
		lcd->setDigitCount(static_cast<int>(precision) + 4);
	}
}

void ValueMonitorWidget::setColor(const QColor &color)
{
	m_color = color.isValid() ? color : Style::getColor(json::theme::content_default);
	applyColor();
}

void ValueMonitorWidget::applyColor()
{
	setStyleSheet(QStringLiteral("scopy--LcdNumber { background-color: transparent; color: %1;"
				     " border: 0px; }")
			      .arg(m_color.name()));
}

void ValueMonitorWidget::setValue(double value) { m_value->display(value); }

void ValueMonitorWidget::setMin(double value) { m_min->display(value); }

void ValueMonitorWidget::setMax(double value) { m_max->display(value); }

void ValueMonitorWidget::setPeakHoldVisible(bool visible) { m_peakHold->setVisible(visible); }

#include "moc_valuemonitorwidget.cpp"
