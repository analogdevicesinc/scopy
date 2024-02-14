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

#include "widgets/popupwidget.h"

#include "stylehelper.h"

using namespace scopy;

PopupWidget::PopupWidget(QWidget *parent)
	: m_tintedOverlay(nullptr)
	, QWidget(parent)
{
	this->initUI();

	QObject::connect(m_continueButton, &QPushButton::clicked, this, &PopupWidget::continueButtonClicked);
	QObject::connect(m_exitButton, &QPushButton::clicked, this, &PopupWidget::exitButtonClicked);

	m_continueButton->setFocus();
}

PopupWidget::~PopupWidget() { delete m_tintedOverlay; }

void PopupWidget::initUI()
{
	this->setObjectName("PopupWidget");
	this->setStyleSheet("");
	this->resize(500, 300);
	auto verticalLayout = new QVBoxLayout(this);
	this->setLayout(verticalLayout);

	auto backgroundWidget = new QWidget(this);
	auto backgroundLayout = new QVBoxLayout(backgroundWidget);
	verticalLayout->addWidget(backgroundWidget);

	m_titleLabel = new QLabel(backgroundWidget);
	m_titleLabel->setObjectName("titleLabel");

	m_descriptionTextBrowser = new QTextBrowser(backgroundWidget);
	m_descriptionTextBrowser->setObjectName("descriptionTextBrowser");

	auto buttonGroup = new QWidget(backgroundWidget);
	auto buttonGroupLayout = new QHBoxLayout(buttonGroup);
	buttonGroupLayout->setContentsMargins(0, 0, 0, 0);
	buttonGroupLayout->setSpacing(10);

	m_continueButton = new QPushButton("Continue", buttonGroup);
	m_continueButton->setObjectName("continueButton");

	m_exitButton = new QPushButton("Exit", buttonGroup);
	m_exitButton->setObjectName("exitButton");

	buttonGroupLayout->addWidget(m_exitButton);
	buttonGroupLayout->addWidget(m_continueButton);

	backgroundLayout->addWidget(m_titleLabel);
	backgroundLayout->addWidget(m_descriptionTextBrowser);
	backgroundLayout->addWidget(buttonGroup);

	backgroundWidget->setLayout(backgroundLayout);

	StyleHelper::TutorialChapterTitleLabel(m_titleLabel, "titleLabel");
	StyleHelper::BlueButton(m_continueButton, "continueButton");
	StyleHelper::BlueButton(m_exitButton, "exitButton");
	StyleHelper::OverlayMenu(this, "popupOverlay");
}

void PopupWidget::setFocusOnContinueButton()
{
	m_continueButton->setAutoDefault(true);
	m_continueButton->setFocus();
}

void PopupWidget::setFocusOnExitButton()
{
	m_exitButton->setAutoDefault(true);
	m_exitButton->setFocus();
}

QString PopupWidget::getDescription() { return m_descriptionTextBrowser->toMarkdown(); }

void PopupWidget::setDescription(const QString &description) { m_descriptionTextBrowser->setMarkdown(description); }

QString PopupWidget::getTitle() { return m_titleLabel->text(); }

void PopupWidget::setTitle(const QString &title) { m_titleLabel->setText(title); }

void PopupWidget::enableTitleBar(bool enable)
{
	if(enable) {
		m_titleLabel->show();
	} else {
		m_titleLabel->hide();
	}
}

QPushButton *PopupWidget::getExitBtn() { return m_exitButton; }

QPushButton *PopupWidget::getContinueBtn() { return m_continueButton; }

void PopupWidget::enableTintedOverlay(bool enable)
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

void PopupWidget::setEnableExternalLinks(bool enable)
{
	m_descriptionTextBrowser->setProperty("openExternalLinks", enable);
}

#include "moc_popupwidget.cpp"
