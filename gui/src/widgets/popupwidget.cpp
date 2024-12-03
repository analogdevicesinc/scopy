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
#include <style.h>
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

PopupWidget::~PopupWidget()
{
	if(m_tintedOverlay != nullptr) {
		m_tintedOverlay->deleteLater();
	}
}

void PopupWidget::initUI()
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	this->setObjectName("PopupWidget");
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

	Style::setStyle(m_continueButton, style::properties::button::basicButton, true, true);
	Style::setStyle(m_exitButton, style::properties::button::basicButton, true, true);
	Style::setStyle(this, style::properties::widget::overlayMenu);

	m_closeButton = nullptr;
	m_closeHover = nullptr;
}

void PopupWidget::enableCloseButton(bool en)
{
	if(en) {
		m_closeButton = new QPushButton(this);
		m_closeButton->setMaximumSize(20, 20);
		m_closeButton->setIcon(Style::getPixmap(":/gui/icons/close_hovered.svg",
							Style::getColor(json::theme::interactive_subtle_idle)));
		Style::setStyle(m_closeButton, style::properties::widget::notInteractive);

		m_closeHover = new HoverWidget(m_closeButton, this, this);
		m_closeHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		m_closeHover->setContentPos(HoverPosition::HP_CENTER);
		m_closeHover->setAnchorOffset(QPoint(-20, 20));
		m_closeHover->setVisible(true);
		m_closeHover->raise();

		connect(m_closeButton, &QPushButton::clicked, this, [=]() { deleteLater(); });
	} else {
		if(m_closeButton != nullptr) {
			delete m_closeButton;
		}
		if(m_closeHover != nullptr) {
			delete m_closeHover;
		}
	}
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
	} else if(m_tintedOverlay != nullptr) {
		delete m_tintedOverlay;
		m_tintedOverlay = nullptr;
	}
}

void PopupWidget::setEnableExternalLinks(bool enable)
{
	m_descriptionTextBrowser->setProperty("openExternalLinks", enable);
}

bool PopupWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::Resize) {
		move(parentWidget()->rect().center() - rect().center());
	}

	return QObject::eventFilter(watched, event);
}

void PopupWidget::enableCenterOnParent(bool enable)
{
	if(enable) {
		move(parentWidget()->rect().center() - rect().center());
		parentWidget()->installEventFilter(this);
	} else {
		parentWidget()->removeEventFilter(this);
	}
}

#include "moc_popupwidget.cpp"
