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
#include "iiostandarditem.h"
#include "style_properties.h"
#include <QVBoxLayout>
#include <style.h>
#include "codegenerator.h"

#define ADD_ICON ":/gui/icons/green_add.svg"
#define REMOVE_ICON ":/gui/icons/orange_close.svg"

using namespace scopy;
using namespace scopy::debugger;

DetailsView::DetailsView(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_guiDetailsView(new GuiDetailsView(this))
	, m_cliDetailsView(new CliDetailsView(this))
	, m_tabWidget(new QTabWidget(this))
	, m_guiView(new QWidget(this))
	, m_iioView(new QWidget(this))
	, m_titleContainer(new QWidget(this))
	, m_titlePath(new PathTitle("Select an IIO item.", this))
	, m_readBtn(new AnimationPushButton(this))
	, m_addToWatchlistBtn(new QPushButton(this))
	, m_uri(uri)
{
	setupUi();
	// Fw the signal
	connect(m_titlePath, &PathTitle::pathSelected, this, &DetailsView::pathSelected);
}

void DetailsView::setupUi()
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);

	m_titleContainer->setLayout(new QHBoxLayout(m_titleContainer));
	m_titleContainer->layout()->setContentsMargins(0, 0, 0, 0);

	m_addToWatchlistBtn->setMaximumSize(25, 25);
	m_addToWatchlistBtn->setDisabled(true);

	m_guiView->setLayout(new QVBoxLayout(m_guiView));
	m_iioView->setLayout(new QVBoxLayout(m_iioView));

	m_guiView->layout()->setContentsMargins(0, 0, 0, 0);
	m_iioView->layout()->setContentsMargins(0, 0, 0, 0);

	m_guiView->layout()->addWidget(m_guiDetailsView);
	m_iioView->layout()->addWidget(m_cliDetailsView);

	m_generatedCodeView = new QWidget(this);
	m_generatedCodeView->setLayout(new QVBoxLayout(m_generatedCodeView));
	m_generatedCodeView->layout()->setContentsMargins(0, 0, 0, 0);
	m_generatedCodeBrowser = new QTextBrowser(this);
	m_generatedCodeView->layout()->addWidget(m_generatedCodeBrowser);

	m_tabWidget->addTab(m_guiView, "GUI View");
	m_tabWidget->addTab(m_iioView, "IIO View");
	m_tabWidget->addTab(m_generatedCodeView, "Libiio Code");
	QTabBar *tabBar = m_tabWidget->tabBar();
	tabBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	m_readBtn->setIcon(Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));
	m_readBtn->setIconSize(QSize(25, 25));
	m_readBtn->setText("Read All");
	m_readBtn->setAutoDefault(true);
	QMovie *movie = new QMovie(":/gui/loading.gif");
	m_readBtn->setAnimation(movie, 20000);
	m_addToWatchlistBtn->setStyleSheet("QPushButton { background-color: transparent; border: 0px; }");

	m_titleContainer->layout()->addWidget(m_titlePath);
	m_titleContainer->layout()->addWidget(m_addToWatchlistBtn);
	m_titleContainer->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
	m_titleContainer->layout()->addWidget(m_readBtn);
	m_titleContainer->layout()->setContentsMargins(0, 0, 0, 0);

	layout()->addWidget(m_titleContainer);

	QFrame *tabContainer = new QFrame(this);
	tabContainer->setLayout(new QVBoxLayout(tabContainer));
	tabContainer->layout()->setContentsMargins(0, 0, 0, 0);
	tabContainer->layout()->addWidget(m_tabWidget);

	layout()->addWidget(tabContainer);

	Style::setBackgroundColor(this, json::theme::background_subtle);
	Style::setBackgroundColor(m_guiView, json::theme::background_primary);
	Style::setBackgroundColor(m_iioView, json::theme::background_primary);

	Style::setStyle(tabContainer, style::properties::debugger::detailsView, true, true);
	Style::setStyle(m_tabWidget, style::properties::debugger::detailsView, true, true);
	Style::setStyle(m_readBtn, style::properties::button::basicButton);
}

void DetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_titlePath->setTitle(item->path());
	m_guiDetailsView->setIIOStandardItem(item);
	m_cliDetailsView->setIIOStandardItem(item);
	this->resetGeneratedCodeView({item});
}

void DetailsView::refreshIIOView() { m_cliDetailsView->refreshView(); }

AnimationPushButton *DetailsView::readBtn() { return m_readBtn; }

QPushButton *DetailsView::addToWatchlistBtn() { return m_addToWatchlistBtn; }

void DetailsView::setAddToWatchlistState(bool add)
{
	m_addToWatchlistBtn->setEnabled(true);
	if(add) {
		m_addToWatchlistBtn->setIcon(Style::getPixmap(ADD_ICON, Style::getColor(json::theme::content_inverse)));
		m_addToWatchlistBtn->setToolTip("Add to Watchlist");
	} else {
		m_addToWatchlistBtn->setIcon(
			Style::getPixmap(REMOVE_ICON, Style::getColor(json::theme::content_inverse)));
		m_addToWatchlistBtn->setToolTip("Remove from Watchlist");
	}
}

void DetailsView::resetGeneratedCodeView(QList<IIOStandardItem *> items)
{
	QList<CodeGenerator::CodeGeneratorRecipe> recipes;
	for(auto item : qAsConst(items)) {
		recipes.append(CodeGenerator::convertToCodeGeneratorRecipe(item, m_uri));
	}

	m_generatedCodeBrowser->setPlainText(CodeGenerator::generateCode(recipes));
}

#include "moc_detailsview.cpp"
