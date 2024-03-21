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

#include <utility>
#include <gui/dynamicWidget.h>
#include <gui/stylehelper.h>

using namespace scopy::swiotrefactor;

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
	StyleHelper::ActiveStoredLabel(m_activeLabel);

	m_storedLabel = new QLabel(this);
	m_storedLabel->setMinimumSize(50, 20);
	StyleHelper::ActiveStoredLabel(m_storedLabel);

	m_titleLabel = new QLabel(this);
	m_titleLabel->setText("Bit" + QString::number(m_id));
	StyleHelper::MenuSmallLabel(m_titleLabel);

	layout->addWidget(m_activeLabel, 0, Qt::AlignCenter);
	layout->addWidget(m_storedLabel, 0, Qt::AlignCenter);
	layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

	installEventFilter(this);
	StyleHelper::FaultsFrame(this);
}

FaultWidget::~FaultWidget() {}

bool FaultWidget::isStored() const { return m_stored; }

void FaultWidget::setStored(bool stored)
{
	FaultWidget::m_stored = stored;
	m_storedLabel->setProperty("high", m_stored);
	m_storedLabel->setStyle(m_storedLabel->style());
}

bool FaultWidget::isActive() const { return m_active; }

void FaultWidget::setActive(bool active)
{
	FaultWidget::m_active = active;
	m_activeLabel->setProperty("high", m_active);
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
	scopy::setDynamicProperty(this, "pressed", pressed);
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
		setPressed(!scopy::getDynamicProperty(this, "pressed"));

		Q_EMIT faultSelected(m_id);
	}

	if(event->type() == QEvent::ToolTip) {
		setToolTip(m_name);
	}

	return QWidget::eventFilter(object, event);
}
