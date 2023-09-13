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
#include "ui_popupwidget.h"

using namespace scopy;

PopupWidget::PopupWidget(QWidget* parent) :
	ui(new Ui::PopupWidget),
	QWidget(parent)
{
	ui->setupUi(this);

	StyleHelper::TitleLabel(ui->titleLabel, "titleLabel");
	StyleHelper::BlueButton(ui->continueButton, "continueButton");
	StyleHelper::BlueButton(ui->exitButton, "exitButton");
	StyleHelper::OverlayMenu(this, "popupOverlay");

	QObject::connect(ui->continueButton, &QPushButton::clicked, this, &PopupWidget::continueButtonClicked);
	QObject::connect(ui->exitButton, &QPushButton::clicked, this, &PopupWidget::exitButtonClicked);

	ui->continueButton->setFocus();
}

void PopupWidget::setFocusOnContinueButton() {
	ui->continueButton->setAutoDefault(true);
	ui->continueButton->setFocus();
}

void PopupWidget::setFocusOnExitButton() {
	ui->exitButton->setAutoDefault(true);
	ui->exitButton->setFocus();
}

QString PopupWidget::getDescription() {
	return ui->descriptionTextBrowser->toMarkdown();
}

void PopupWidget::setDescription(const QString& description) {
	ui->descriptionTextBrowser->setMarkdown(description);
}

QString PopupWidget::getTitle() {
	return ui->titleLabel->text();
}

void PopupWidget::setTitle(const QString &title) {
	ui->titleLabel->setText(title);
}

void PopupWidget::enableTitleBar(bool enable) {
	if (enable) {
		ui->titleLabel->show();
	} else {
		ui->titleLabel->hide();
	}
}

QString PopupWidget::getContinueButtonText() {
	return ui->continueButton->text();
}

void PopupWidget::setContinueButtonText(const QString& text) {
	ui->continueButton->setText(text);
}

QString PopupWidget::getExitButtonText() {
	return ui->exitButton->text();
}

void PopupWidget::setExitButtonText(const QString& text) {
	ui->exitButton->setText(text);
}

#include "moc_popupwidget.cpp"
