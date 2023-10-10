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

#include "faultwidget.h"

#include "core/logging_categories.h"

#include "ui_faultwidget.h"

#include <utility>

using namespace scopy::swiot;

FaultWidget::FaultWidget(unsigned int id, QString name, QString faultExplanation, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::FaultWidget)
	, m_stored(false)
	, m_active(false)
	, m_pressed(false)
	, m_id(id)
	, m_name(std::move(name))
	, m_faultExplanation(std::move(faultExplanation))
{
	ui->setupUi(this);

	installEventFilter(this);

	this->ui->title->setText("Bit" + QString::number(this->m_id));

	this->ui->mainFrame->setMinimumSize(70, 90);
}

FaultWidget::~FaultWidget() { delete ui; }

bool FaultWidget::isStored() const { return m_stored; }

void FaultWidget::setStored(bool stored)
{
	FaultWidget::m_stored = stored;
	this->ui->stored->setProperty("high", this->m_stored);
	this->ui->stored->setStyle(this->ui->stored->style());
}

bool FaultWidget::isActive() const { return m_active; }

void FaultWidget::setActive(bool active)
{
	FaultWidget::m_active = active;
	this->ui->active->setProperty("high", m_active);
	this->ui->active->setStyle(this->ui->active->style());
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
	scopy::setDynamicProperty(this->ui->mainFrame, "pressed", pressed);
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
		this->setPressed(!scopy::getDynamicProperty(this->ui->mainFrame, "pressed"));

		Q_EMIT faultSelected(this->m_id);
	}

	if(event->type() == QEvent::ToolTip) {
		this->setToolTip(this->m_name);
	}

	return QWidget::eventFilter(object, event);
}
