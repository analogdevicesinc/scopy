/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "faults/faultwidget.h"

#include <style.h>
#include <utility>
#include <gui/dynamicWidget.h>
#include <gui/stylehelper.h>
#include "style_properties.h"

using namespace scopy::swiot;

FaultWidget::FaultWidget(unsigned int id, QString name, QString faultExplanation, QWidget *parent)
	: QFrame(parent)
	, m_id(id)
	, m_name(std::move(name))
	, m_faultExplanation(std::move(faultExplanation))
{
	setFrameShape(StyledPanel);
	setFrameShadow(Raised);

	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	m_activeLabel = new QLabel(this);
	m_activeLabel->setMinimumSize(50, 20);
	Style::setStyle(m_activeLabel, style::properties::swiot::activeStoredLabel, "low", true);

	m_storedLabel = new QLabel(this);
	m_storedLabel->setMinimumSize(50, 20);
	Style::setStyle(m_storedLabel, style::properties::swiot::activeStoredLabel, "low", true);

	m_titleLabel = new QLabel(this);
	m_titleLabel->setText("Bit" + QString::number(m_id));
	Style::setStyle(m_titleLabel, style::properties::label::menuSmall);

	layout->addWidget(m_activeLabel, 0, Qt::AlignCenter);
	layout->addWidget(m_storedLabel, 0, Qt::AlignCenter);
	layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

	installEventFilter(this);
}

FaultWidget::~FaultWidget() {}

bool FaultWidget::isStored() const { return m_stored; }

void FaultWidget::setStored(bool stored)
{
	FaultWidget::m_stored = stored;
	Style::setStyle(m_storedLabel, style::properties::swiot::activeStoredLabel, stored ? "high" : "low", true);
	m_storedLabel->setStyle(m_storedLabel->style());
}

bool FaultWidget::isActive() const { return m_active; }

void FaultWidget::setActive(bool active)
{
	FaultWidget::m_active = active;
	Style::setStyle(m_activeLabel, style::properties::swiot::activeStoredLabel, active ? "high" : "low", true);
	m_activeLabel->setStyle(m_activeLabel->style());
}

const QString &FaultWidget::getName() const { return m_name; }

void FaultWidget::setName(const QString &name) { FaultWidget::m_name = name; }

const QString &FaultWidget::getFaultExplanation() const { return m_faultExplanation; }

void FaultWidget::setFaultExplanation(const QString &faultExplanation)
{
	FaultWidget::m_faultExplanation = faultExplanation;
}

void FaultWidget::setFaultExplanationOptions(QJsonObject options) { m_faultExplanationOptions = options; }

unsigned int FaultWidget::getId() const { return m_id; }

void FaultWidget::setId(unsigned int id) { FaultWidget::m_id = id; }

bool FaultWidget::isPressed() const { return m_pressed; }

void FaultWidget::setPressed(bool pressed)
{
	FaultWidget::m_pressed = pressed;
	Style::setStyle(this, style::properties::swiot::faultsFrame, pressed);
}

void FaultWidget::specialFaultUpdated(int index, QString channelFunction)
{
	if(index == m_id) {
		QString option = m_faultExplanationOptions[channelFunction].toString();
		if(option.isEmpty()) {
			option = "channel configured as: " + channelFunction;
		}
		setFaultExplanation(option);
		Q_EMIT specialFaultExplanationChanged(m_id, m_faultExplanation);
	}
}

bool FaultWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress) {
		setPressed(!scopy::getDynamicProperty(this, style::properties::swiot::faultsFrame));

		Q_EMIT faultSelected(m_id);
	}

	if(event->type() == QEvent::ToolTip) {
		setToolTip(m_name);
	}

	return QWidget::eventFilter(object, event);
}

#include "moc_faultwidget.cpp"
