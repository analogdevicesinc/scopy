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

#include "scopyaboutpage.h"

#include "widgets/hoverwidget.h"
#include "widgets/pagenavigationwidget.h"

#include <QLabel>
#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <style.h>

#include <stylehelper.h>
#include <verticaltabwidget.h>

using namespace scopy;
ScopyAboutPage::ScopyAboutPage(QWidget *parent)
	: QWidget(parent)
	, tabWidget(new VerticalTabWidget(this))
	, layout(new QVBoxLayout(this))
{
	initUI();
	addHorizontalTab(buildPage(QString("qrc:/about.html")), "Scopy");
}

void ScopyAboutPage::initUI()
{
	layout->setMargin(0);
	layout->setSpacing(0);
	this->setLayout(layout);
	layout->addWidget(tabWidget);

	StyleHelper::TabWidgetEastMenu(tabWidget, "aboutPageTable");
}

QWidget *ScopyAboutPage::buildPage(QString src)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	QTextBrowser *browser = new QTextBrowser(page);
	browser->setOpenExternalLinks(true);

	lay->addWidget(browser);
	lay->setMargin(0);
	initNavigationWidget(browser);
	browser->setStyleSheet("background-color: " + Style::getAttribute(json::theme::content_inverse));

	if(QFile::exists(QString(src).replace("qrc:/", ":/"))) {
		browser->setSource(src);
	} else {
		browser->setMarkdown(src);
	}

	return page;
}

void ScopyAboutPage::initNavigationWidget(QTextBrowser *browser)
{
	PageNavigationWidget *navWidget = new PageNavigationWidget(true, false, this);
	QPushButton *homeButton = navWidget->getHomeBtn();
	connect(homeButton, SIGNAL(clicked()), browser, SLOT(home()));

	QPushButton *backwardButton = navWidget->getBackwardBtn();
	backwardButton->setEnabled(false);
	connect(backwardButton, SIGNAL(clicked()), browser, SLOT(backward()));
	connect(browser, &QTextBrowser::backwardAvailable, backwardButton,
		[=](bool available) { backwardButton->setEnabled(available); });

	QPushButton *forwardButton = navWidget->getForwardBtn();
	forwardButton->setEnabled(false);
	connect(forwardButton, SIGNAL(clicked()), browser, SLOT(forward()));
	connect(browser, &QTextBrowser::forwardAvailable, forwardButton,
		[=](bool available) { forwardButton->setEnabled(available); });

	HoverWidget *hover = new HoverWidget(navWidget, browser, browser);
	hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	hover->setAnchorOffset(QPoint(-10, 0));
	hover->show();
}

void ScopyAboutPage::addHorizontalTab(QWidget *w, QString text)
{
	tabWidget->addTab(w, text);
}

ScopyAboutPage::~ScopyAboutPage() {}

#include "moc_scopyaboutpage.cpp"
