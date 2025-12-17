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

#include "genalyzerpanel.h"
#include <style.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QFont>
#include <QTextBrowser>
#include <QDockWidget>
#include <QApplication>
#include <QLabel>

using namespace scopy;

GenalyzerChannelDisplay::GenalyzerChannelDisplay(const QString &channelName, QColor channelColor, QWidget *parent)
	: QTextBrowser(parent)
	, m_channelName(channelName)
	, m_channelColor(channelColor)
{
	setObjectName("GenalyzerChannelDisplay");
	setReadOnly(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QFont font;
	font.setPixelSize(Style::getDimension(json::global::font_size));
	setFont(font);

	setChannelColor(channelColor);
}

void GenalyzerChannelDisplay::updateResults(size_t results_size, char **rkeys, double *rvalues)
{
	QScrollBar *vScrollBar = verticalScrollBar();
	int scrollPosition = vScrollBar->value();

	setPlainText(formatResultsText(results_size, rkeys, rvalues));

	vScrollBar->setValue(scrollPosition);
}

void GenalyzerChannelDisplay::setChannelColor(QColor color)
{
	m_channelColor = color;
	setStyleSheet("QTextBrowser { color: " + color.name() + "; border: none; background: transparent; }");
}

QString GenalyzerChannelDisplay::channelName() const { return m_channelName; }

QString GenalyzerChannelDisplay::formatResultsText(size_t results_size, char **rkeys, double *rvalues)
{
	if(results_size == 0) {
		return "No results";
	}

	QString result;
	result.reserve(results_size * 30);

	const int keyWidth = 18;
	const int valueWidth = 12;

	for(size_t i = 0; i < results_size; i++) {
		QString key = QString(rkeys[i]) + ":";
		QString value = QString::number(rvalues[i], 'f', 3);

		QString line = key.leftJustified(keyWidth) + value.rightJustified(valueWidth);
		result += line + '\n';
	}

	return result;
}

GenalyzerPanel::GenalyzerPanel(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setMinimumWidth(200);
	setMaximumWidth(400);
	Style::setBackgroundColor(this, json::theme::background_subtle);

	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setSpacing(0);

	m_embeddedMainWindow = new QMainWindow(this);
	m_embeddedMainWindow->setWindowFlags(Qt::Widget);

	QWidget *centralWidget = new QWidget(m_embeddedMainWindow);
	centralWidget->setFixedSize(0, 0);
	m_embeddedMainWindow->setCentralWidget(centralWidget);

	m_embeddedMainWindow->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks |
					     QMainWindow::VerticalTabs);

	m_mainLayout->addWidget(m_embeddedMainWindow);
}

GenalyzerPanel::~GenalyzerPanel() { clear(); }

QDockWidget *GenalyzerPanel::findOrCreateChannelDock(const QString &channelName, QColor channelColor)
{
	if(m_channelDocks.contains(channelName)) {
		return m_channelDocks[channelName];
	}

	GenalyzerChannelDisplay *display = new GenalyzerChannelDisplay(channelName, channelColor, this);
	m_channelDisplays[channelName] = display;

	QDockWidget *dock = new QDockWidget(channelName, m_embeddedMainWindow);
	dock->setWidget(display);

	dock->setFeatures(QDockWidget::DockWidgetMovable);
	dock->setAllowedAreas(Qt::TopDockWidgetArea);
	dock->setFeatures(QDockWidget::DockWidgetFloatable);

	dock->setMinimumHeight(100);
	display->setMinimumHeight(25);

	display->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	if(m_channelDocks.isEmpty()) {
		m_embeddedMainWindow->addDockWidget(Qt::TopDockWidgetArea, dock);
	} else {
		QDockWidget *lastDock = m_channelDocks.values().last();
		m_embeddedMainWindow->splitDockWidget(lastDock, dock, Qt::Vertical);
	}

	m_channelDocks[channelName] = dock;

	return dock;
}

void GenalyzerPanel::updateResults(const QString &channelName, QColor channelColor, size_t results_size, char **rkeys,
				   double *rvalues)
{
	findOrCreateChannelDock(channelName, channelColor);

	if(m_channelDisplays.contains(channelName)) {
		m_channelDisplays[channelName]->updateResults(results_size, rkeys, rvalues);
	}
}

void GenalyzerPanel::clearChannel(const QString &channelName)
{
	if(m_channelDocks.contains(channelName)) {
		QDockWidget *dock = m_channelDocks.take(channelName);
		m_embeddedMainWindow->removeDockWidget(dock);
		dock->deleteLater();
	}

	if(m_channelDisplays.contains(channelName)) {
		m_channelDisplays.remove(channelName);
	}
}

void GenalyzerPanel::clear()
{
	for(auto it = m_channelDocks.begin(); it != m_channelDocks.end(); ++it) {
		m_embeddedMainWindow->removeDockWidget(it.value());
		it.value()->deleteLater();
	}
	m_channelDocks.clear();
	m_channelDisplays.clear();
}

void GenalyzerPanel::setChannelVisible(const QString &channelName, bool visible)
{
	if(m_channelDocks.contains(channelName)) {
		m_channelDocks[channelName]->setVisible(visible);
	}
}

#include "moc_genalyzerpanel.cpp"
