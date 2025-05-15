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

#include "whatsnewoverlay.h"

#include <QCheckBox>
#include <QDir>
#include <QPushButton>
#include <qboxlayout.h>
#include <style.h>
#include <QDebug>
#include <QScrollArea>
#include <preferenceshelper.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy;

WhatsNewOverlay::WhatsNewOverlay(QWidget *parent)
	: m_tintedOverlay(nullptr)
	, QWidget{parent}
{
	// main layout vertical
	this->resize(400, 600);
	QVBoxLayout *mainLayout = new QVBoxLayout();
	this->setLayout(mainLayout);

	m_carouselWidget = new QStackedWidget(this);

	// bottom of main layout has control widget

	QWidget *overlayControlWidget = new QWidget(this);
	QVBoxLayout *overlayControlWidgetLayout = new QVBoxLayout(overlayControlWidget);

	QWidget *optionsoverlayControlWidget = new QWidget(overlayControlWidget);
	overlayControlWidgetLayout->addWidget(optionsoverlayControlWidget);
	QHBoxLayout *optionsControlLayout = new QHBoxLayout(optionsoverlayControlWidget);
	optionsoverlayControlWidget->setLayout(optionsControlLayout);

	QCheckBox *showAgain = new QCheckBox("Show this again", optionsoverlayControlWidget);
	showAgain->setChecked(true);
	optionsControlLayout->addWidget(showAgain);

	connect(showAgain, &QCheckBox::toggled, this, [=](bool en) {
		Preferences *p = Preferences::GetInstance();
		p->set("general_show_whats_new", en);
	});

	// version picker
	m_versionCb = new QComboBox(optionsoverlayControlWidget);

	connect(m_versionCb, qOverload<int>(&QComboBox::currentIndexChanged), this,
		[=](int idx) { m_carouselWidget->setCurrentIndex(idx); });

	QPushButton *okButton = new QPushButton("Ok", optionsoverlayControlWidget);
	connect(okButton, &QPushButton::clicked, this, [=]() { this->deleteLater(); });
	Style::setStyle(okButton, style::properties::button::basicButton, true, true);
	optionsControlLayout->addWidget(okButton);

	mainLayout->addWidget(m_carouselWidget);
	mainLayout->addWidget(m_versionCb);
	mainLayout->addWidget(overlayControlWidget);
	Style::setBackgroundColor(this, json::theme::background_primary);

	initCarousel();
	enableTintedOverlay(true);
}

WhatsNewOverlay::~WhatsNewOverlay()
{
	// TODO
	if(m_tintedOverlay != nullptr) {
		m_tintedOverlay->deleteLater();
	}
}

void WhatsNewOverlay::showOverlay()
{
	raise();
	show();
}

void WhatsNewOverlay::enableTintedOverlay(bool enable)
{
	if(enable) {
		delete m_tintedOverlay;

		m_tintedOverlay = new gui::TintedOverlay(parentWidget());
		m_tintedOverlay->show();
		raise();
		show();
	} else {
		delete m_tintedOverlay;
		m_tintedOverlay = nullptr;
	}
}

void WhatsNewOverlay::initCarousel()
{
	QDir dir(":/whatsnew");

	// Get a list of all entries in the directory
	QFileInfoList entries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);

	// Iterate through the list and print the folder names

	for(int i = entries.count() - 1; i >= 0; i--) {
		QFileInfo fileInfo = entries.at(i);
		if(fileInfo.isDir()) {
			// add option to combo
			m_versionCb->addItem(fileInfo.fileName());
			// create page and button for page
			generateVersionPage(fileInfo.absoluteFilePath());
		}
	}
}

void WhatsNewOverlay::generateVersionPage(QString filePath)
{
	QWidget *carouselContent = new QWidget(m_carouselWidget);
	QVBoxLayout *carouselContentLayout = new QVBoxLayout(carouselContent);
	carouselContentLayout->setMargin(2);
	carouselContentLayout->setSpacing(0);

	QStackedWidget *versionCarouselWithControls = new QStackedWidget(m_carouselWidget);
	carouselContentLayout->addWidget(versionCarouselWithControls, 9);

	QDir versionDir(filePath);
	const QFileInfoList fileList = versionDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

	for(const QFileInfo &page : fileList) {

		QWidget *scrollWidget = new QWidget();
		scrollWidget->setLayout(new QVBoxLayout());
		QScrollArea *scrollArea = new QScrollArea();
		scrollArea->setWidgetResizable(true);
		scrollArea->setWidget(scrollWidget);

		QTextBrowser *htmlPage = new QTextBrowser(scrollWidget);
		QString filePath(page.absoluteFilePath());
		htmlPage->setHtml(getHtmlPageContent(filePath));
		htmlPage->setProperty("openExternalLinks", true);

		scrollWidget->layout()->addWidget(htmlPage);
		versionCarouselWithControls->addWidget(scrollArea);
		Style::setStyle(scrollWidget, style::properties::widget::border_interactive);
	}

	int numberOfPages = fileList.count();

	if(numberOfPages > 1) {
		// carousel control contains buttons for changeing info page
		QWidget *carouselControlWidget = new QWidget(carouselContent);

		QButtonGroup *m_carouselButtons = new QButtonGroup(carouselControlWidget);
		m_carouselButtons->setExclusive(true);

		QHBoxLayout *carouselControlayout = new QHBoxLayout(carouselControlWidget);
		carouselControlWidget->setLayout(carouselControlayout);
		carouselControlayout->setContentsMargins(0, 0, 0, 4);

		QPushButton *previous = new QPushButton(carouselControlWidget);

		StyleHelper::BlueIconButton(previous,
					    Style::getPixmap(":/gui/icons/handle_left_arrow.svg",
							     Style::getColor(json::theme::content_inverse)),
					    "previousButton");

		carouselControlayout->addWidget(previous);
		carouselControlayout->addSpacerItem(
			new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

		for(int i = 0; i < numberOfPages; i++) {

			QPushButton *htmlPageButton = new QPushButton("", carouselControlWidget);
			htmlPageButton->setCheckable(true);

			if(i == 0) {
				htmlPageButton->setChecked(true);
			}

			connect(htmlPageButton, &QPushButton::toggled, this, [=](bool toggled) {
				if(toggled) {
					versionCarouselWithControls->setCurrentIndex(i);
				}
			});

			carouselControlayout->addWidget(htmlPageButton, 0, Qt::AlignBottom);
			m_carouselButtons->addButton(htmlPageButton, i);
			Style::setStyle(htmlPageButton, style::properties::button::whatsNewButton, true, true);
			htmlPageButton->setFixedHeight(Style::getAttribute(json::global::radius_2).toInt());
		}

		QPushButton *next = new QPushButton(carouselControlWidget);
		StyleHelper::BlueIconButton(next,
					    Style::getPixmap(":/gui/icons/handle_right_arrow.svg",
							     Style::getColor(json::theme::content_inverse)),
					    "nextButton");

		carouselControlayout->addSpacerItem(
			new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
		carouselControlayout->addWidget(next);

		connect(previous, &QPushButton::clicked, this, [=]() {
			int currentIndex = m_carouselButtons->checkedId();
			int previousIndex = (currentIndex - 1 + numberOfPages) % numberOfPages;
			m_carouselButtons->button(previousIndex)->setChecked(true);
		});

		connect(next, &QPushButton::clicked, this, [=]() {
			int currentIndex = m_carouselButtons->checkedId();
			int nextIndex = (currentIndex + 1) % numberOfPages;
			m_carouselButtons->button(nextIndex)->setChecked(true);
		});

		carouselContentLayout->addWidget(carouselControlWidget, 1);
	}

	m_carouselWidget->addWidget(carouselContent);
}

QString WhatsNewOverlay::getHtmlPageContent(QString fileName)
{
	QFile file(fileName);
	if(file.open(QIODevice::ReadOnly)) {
		QString text = QString(file.readAll());
		file.close();
		return text;
	}
	return "";
}

#include "moc_whatsnewoverlay.cpp"
