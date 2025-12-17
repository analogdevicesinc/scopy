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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDockWidget>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QClipboard>

using namespace scopy;

GenalyzerChannelDisplay::GenalyzerChannelDisplay(const QString &channelName, QColor channelColor, QWidget *parent)
	: QTableWidget(parent)
	, m_channelName(channelName)
	, m_channelColor(channelColor)
{
	setObjectName("GenalyzerChannelDisplay");

	setColumnCount(2);
	setAlternatingRowColors(false);
	setShowGrid(false);
	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setFocusPolicy(Qt::NoFocus);

	horizontalHeader()->hide();
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(20);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setChannelColor(channelColor);
}

void GenalyzerChannelDisplay::updateResults(size_t results_size, char **rkeys, double *rvalues)
{
	setUpdatesEnabled(false);

	int currentRowCount = rowCount();
	int newRowCount = static_cast<int>(results_size);

	if(newRowCount != currentRowCount) {
		setRowCount(newRowCount);
	}

	for(int i = 0; i < newRowCount; ++i) {
		QTableWidgetItem *keyItem = item(i, 0);
		QTableWidgetItem *valueItem = item(i, 1);

		if(!keyItem) {
			keyItem = new QTableWidgetItem();
			keyItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			keyItem->setForeground(m_channelColor);
			setItem(i, 0, keyItem);
		}

		if(!valueItem) {
			valueItem = new QTableWidgetItem();
			valueItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			valueItem->setForeground(m_channelColor);
			setItem(i, 1, valueItem);
		}

		keyItem->setText(QString(rkeys[i]) + ":");
		valueItem->setText(QString::number(rvalues[i], 'f', 3));
	}

	setUpdatesEnabled(true);
}

void GenalyzerChannelDisplay::setChannelColor(QColor color)
{
	m_channelColor = color;
	setStyleSheet("QTableWidget { border: none; background: transparent; }"
		      "QTableWidget::item { border: none; padding: 2px; }");

	for(int i = 0; i < rowCount(); ++i) {
		if(QTableWidgetItem *keyItem = item(i, 0)) {
			keyItem->setForeground(color);
		}
		if(QTableWidgetItem *valueItem = item(i, 1)) {
			valueItem->setForeground(color);
		}
	}
}

QString GenalyzerChannelDisplay::channelName() const { return m_channelName; }

QString GenalyzerChannelDisplay::getTableContent() const
{
	QString content;
	for(int i = 0; i < rowCount(); ++i) {
		QTableWidgetItem *keyItem = item(i, 0);
		QTableWidgetItem *valueItem = item(i, 1);
		if(keyItem && valueItem) {
			content += keyItem->text() + "\t" + valueItem->text() + "\n";
		}
	}
	return content;
}

GenalyzerPanel::GenalyzerPanel(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setMinimumWidth(250);
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

	QWidget *containerWidget = new QWidget();
	QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);
	containerLayout->setMargin(2);
	containerLayout->setSpacing(2);

	GenalyzerChannelDisplay *display = new GenalyzerChannelDisplay(channelName, channelColor, containerWidget);
	m_channelDisplays[channelName] = display;
	display->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	QPushButton *copyButton = new QPushButton("Copy to clipboard", containerWidget);
	QFont font;
	font.setPixelSize(Style::getDimension(json::global::font_size_0_5));
	copyButton->setFont(font);
	copyButton->setFocusPolicy(Qt::NoFocus);
	connect(copyButton, &QPushButton::clicked, [display]() {
		QString content = display->getTableContent();
		QApplication::clipboard()->setText(content);
	});

	containerLayout->addWidget(display);
	containerLayout->addWidget(copyButton);

	QDockWidget *dock = new QDockWidget(channelName, m_embeddedMainWindow);
	dock->setWidget(containerWidget);

	dock->setFeatures(QDockWidget::DockWidgetMovable);
	dock->setFeatures(QDockWidget::DockWidgetFloatable);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);

	dock->setMinimumHeight(100);

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
