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

#include "iiodebuglogger.h"
#include "debuggerloggingcategories.h"
#include <QVBoxLayout>
#include <QFont>
#include <style.h>

using namespace scopy::debugger;

IIODebugLogger::IIODebugLogger(QWidget *parent)
	: QFrame(parent)
	, m_textBrowser(new QTextBrowser(this))
{
	setupUi();
}

void IIODebugLogger::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_textBrowser);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_textBrowser->setTabStopDistance(30);
	QFont mono("Monospace");
	mono.setStyleHint(QFont::Monospace);
	m_textBrowser->setFont(mono);
	m_textBrowser->setReadOnly(true);
	Style::setStyle(m_textBrowser, style::properties::widget::textBrowser);
}

void IIODebugLogger::appendLog(QString log)
{
	qDebug(CAT_DEBUGGERIIOMODEL) << "Adding to log: " << log;
	m_textBrowser->append(log);
}

#include "moc_iiodebuglogger.cpp"
