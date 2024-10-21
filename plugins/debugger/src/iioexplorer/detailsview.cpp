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

#include "detailsview.h"
#include <QVBoxLayout>
#include <style.h>

#define ADD_ICON ":/gui/icons/green_add.svg"
#define REMOVE_ICON ":/gui/icons/orange_close.svg"

using namespace scopy::debugger;

DetailsView::DetailsView(QWidget *parent)
	: QWidget(parent)
	, m_titlePath(new PathTitle("Select an IIO item.", this))
	, m_guiDetailsView(new GuiDetailsView(this))
	, m_cliDetailsView(new CliDetailsView(this))
	, m_tabWidget(new QTabWidget(this))
	, m_guiView(new QWidget(this))
	, m_iioView(new QWidget(this))
	, m_readBtn(new QPushButton(this))
	, m_addToWatchlistBtn(new QPushButton(this))
	, m_titleContainer(new QWidget(this))
{
	setupUi();
	// Fw the signal
	connect(m_titlePath, &PathTitle::pathSelected, this, &DetailsView::pathSelected);
}

void DetailsView::setupUi()
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 6, 0, 0);

	m_titleContainer->setLayout(new QHBoxLayout(m_titleContainer));
	m_titleContainer->layout()->setContentsMargins(0, 0, 0, 0);

	m_readBtn->setMaximumWidth(90);
	m_addToWatchlistBtn->setMaximumSize(25, 25);
	m_addToWatchlistBtn->setDisabled(true);

	m_guiView->setLayout(new QVBoxLayout(m_guiView));
	m_iioView->setLayout(new QVBoxLayout(m_iioView));

	m_guiView->layout()->setContentsMargins(0, 0, 0, 0);
	m_iioView->layout()->setContentsMargins(0, 0, 0, 0);

	m_guiView->layout()->addWidget(m_guiDetailsView);
	m_iioView->layout()->addWidget(m_cliDetailsView);

	m_tabWidget->addTab(m_guiView, "GUI View");
	m_tabWidget->addTab(m_iioView, "IIO View");
	QTabBar *tabBar = m_tabWidget->tabBar();
	tabBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	StyleHelper::TabWidgetBarUnderline(m_tabWidget, "DetailsTabWidget");
	m_readBtn->setProperty("blue_button", true);
	m_readBtn->setIcon(QIcon(":/gui/icons/refresh.svg"));
	m_readBtn->setIconSize(QSize(25, 25));
	m_readBtn->setText("Refresh");
	m_readBtn->setMinimumHeight(35);
	m_addToWatchlistBtn->setStyleSheet("QPushButton { background-color: transparent; border: 0px; }");

	m_titleContainer->layout()->addWidget(m_titlePath);
	m_titleContainer->layout()->addWidget(m_addToWatchlistBtn);
	m_titleContainer->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	m_titleContainer->layout()->addWidget(m_readBtn);

	layout()->addWidget(m_titleContainer);
	layout()->addWidget(m_tabWidget);
}

void DetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_titlePath->setTitle(item->path());
	m_guiDetailsView->setIIOStandardItem(item);
	m_cliDetailsView->setIIOStandardItem(item);
}

void DetailsView::refreshIIOView() { m_cliDetailsView->refreshView(); }

QPushButton *DetailsView::readBtn() { return m_readBtn; }

QPushButton *DetailsView::addToWatchlistBtn() { return m_addToWatchlistBtn; }

void DetailsView::setAddToWatchlistState(bool add)
{
	m_addToWatchlistBtn->setEnabled(true);
	if(add) {
		m_addToWatchlistBtn->setIcon(Style::getPixmap(ADD_ICON, Style::getColor(json::theme::content_inverse)));
		m_addToWatchlistBtn->setToolTip("Add to Watchlist");
	} else {
		m_addToWatchlistBtn->setIcon(Style::getPixmap(REMOVE_ICON, Style::getColor(json::theme::content_inverse)));
		m_addToWatchlistBtn->setToolTip("Remove from Watchlist");
	}
}

#include "moc_detailsview.cpp"
