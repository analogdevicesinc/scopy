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

#include "statusmessage.h"
#include <QLabel>
#include <QDateTime>

using namespace scopy;

// ---------------------------------- STATUS_MESSAGE_TEXT ----------------------------------

StatusMessageText::StatusMessageText(QString text, int ms, QWidget *parent)
	: m_ms(ms)
	, m_text(text)
{
	setParent(parent);
	prependDateTime();

	auto *label = new QLabel(m_text, this);
	label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	label->setStyleSheet("border: none;");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	label->setMaximumHeight(15);
	m_widget = label;
}

StatusMessageText::~StatusMessageText() { delete m_widget; }

QString StatusMessageText::getText() { return m_text; }

QWidget *StatusMessageText::getWidget() { return m_widget; }

int StatusMessageText::getDisplayTime() { return m_ms; }

void StatusMessageText::prependDateTime()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	QString formattedTime = dateTime.toString(TIMESTAMP_FORMAT);
	m_text = formattedTime + " - " + m_text;
}

// ---------------------------------- STATUS_MESSAGE_WIDGET ----------------------------------

StatusMessageWidget::StatusMessageWidget(QWidget *widget, QString description, int ms, QWidget *parent)
	: m_ms(ms)
	, m_text(description)
	, m_widget(widget)
{
	setParent(parent);
}

StatusMessageWidget::~StatusMessageWidget()
{
	// the widget is not permanently displayed, so the responsibility of deleting it belongs to this class
	if(m_ms != -1) {
		delete m_widget;
		m_widget = nullptr;
	}
}

QString StatusMessageWidget::getText() { return m_text; }

QWidget *StatusMessageWidget::getWidget() { return m_widget; }

int StatusMessageWidget::getDisplayTime() { return m_ms; }

#include "moc_statusmessage.cpp"
