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

#include "customSwitch.h"
#include "qcoreevent.h"
#include "qvariant.h"
#include <style.h>
#include <QHBoxLayout>

using namespace scopy;

CustomSwitch::CustomSwitch(QWidget *parent)
	: QPushButton(parent)
	, m_onLabel(new QLabel("On", this))
	, m_offLabel(new QLabel("Off", this))
{
	setCheckable(true);
	init();

	connect(this, &QPushButton::setChecked, this, [this](bool checked) { update(); });
	update();
}

CustomSwitch::CustomSwitch(QString on, QString off, QWidget *parent)
	: CustomSwitch(parent)
{
	setOnText(on);
	setOffText(off);
}

CustomSwitch::~CustomSwitch() {}

void CustomSwitch::init()
{
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_onLabel->setAlignment(Qt::AlignCenter);
	Style::setStyle(m_onLabel, style::widget::customSwitchLeft);
	m_offLabel->setAlignment(Qt::AlignCenter);
	Style::setStyle(m_offLabel, style::widget::customSwitchRight);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(m_onLabel);
	layout->addWidget(m_offLabel);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);
}

void CustomSwitch::paintEvent(QPaintEvent *event)
{
	m_offLabel->move(m_onLabel->width() - Style::getDimension(json::global::border_width), m_offLabel->pos().y());
	if(isChecked()) {
		m_onLabel->raise();
		m_onLabel->setStyleSheet("border-color: " + Style::getAttribute(json::theme::highlight_color));
		m_offLabel->setStyleSheet("border-color: " + Style::getAttribute(json::theme::highlight_secondary));
	} else {
		m_offLabel->raise();
		m_offLabel->setStyleSheet("border-color: " + Style::getAttribute(json::theme::highlight_color));
		m_onLabel->setStyleSheet("border-color: " + Style::getAttribute(json::theme::highlight_secondary));
	}
}

bool CustomSwitch::event(QEvent *e)
{
	if(e->type() == QEvent::DynamicPropertyChange) {
		QDynamicPropertyChangeEvent *const propEvent = static_cast<QDynamicPropertyChangeEvent *>(e);
		QString propName = propEvent->propertyName();
		if(propName == "leftText" && property("leftText").isValid())
			setOnText(property("leftText").toString());
		if(propName == "rightText" && property("rightText").isValid())
			setOffText(property("rightText").toString());
	}
	return QPushButton::event(e);
}

void CustomSwitch::setOnText(QString text)
{
	m_onLabel->setText(text);
	update();
}

void CustomSwitch::setOffText(QString text)
{
	m_offLabel->setText(text);
	update();
}

QSize CustomSwitch::sizeHint() const
{
	QSize size = layout()->sizeHint();
	size.rwidth() -= Style::getDimension(json::global::border_width);
	return size;
}

void CustomSwitch::update()
{
	QPushButton::update();
	setMaximumSize(sizeHint());
}

#include "moc_customSwitch.cpp"
