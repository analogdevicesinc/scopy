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

#include "connectionloadingbar.h"
#include <QHBoxLayout>
#include <stylehelper.h>

using namespace scopy;

ConnectionLoadingBar::ConnectionLoadingBar(QWidget *parent)
	: QWidget{parent}
	, m_currentPluginName("")
	, m_pluginCount(0)
	, m_noPlugins(0)
{
	setupUi();
}

void ConnectionLoadingBar::setCurrentPlugin(QString pluginName)
{
	m_currentPluginName = pluginName;
	m_loadingLabel->setText(QString("Loading %1 plugin").arg(m_currentPluginName));
}

void ConnectionLoadingBar::addProgress(int progress)
{
	m_progressBar->setValue(m_progressBar->value() + progress);
	++m_pluginCount;
	m_pluginCountLabel->setText(QString("%1/%2").arg(m_pluginCount).arg(m_noPlugins));
}

void ConnectionLoadingBar::setProgressBarMaximum(int maximum)
{
	m_noPlugins = maximum;
	m_progressBar->setMaximum(m_noPlugins);
}

void ConnectionLoadingBar::setupUi()
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);

	m_loadingLabel = new QLabel("", this);
	m_pluginCountLabel = new QLabel("", this);
	m_progressBar = new QProgressBar(this);
	m_progressBar->setMinimum(0);
	m_progressBar->setTextVisible(false);
	m_progressBar->setMaximumHeight(10);
	m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_progressBar->setValue(0);
	QString style = QString("QProgressBar::chunk {background-color: %1;}").arg(StyleHelper::getColor("ScopyBlue"));
	setStyleSheet(style);

	layout()->addWidget(m_loadingLabel);
	layout()->addWidget(m_pluginCountLabel);
	layout()->addWidget(m_progressBar);
}

#include "moc_connectionloadingbar.cpp"
