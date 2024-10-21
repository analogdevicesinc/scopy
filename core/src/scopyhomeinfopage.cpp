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

#include "scopyhomeinfopage.h"
#include "style.h"

#include "ui_scopyhomeinfopage.h"

#include <QDesktopServices>

#include <stylehelper.h>

using namespace scopy;

ScopyHomeInfoPage::ScopyHomeInfoPage(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ScopyHomeInfoPage)
{
	ui->setupUi(this);
	Style::setStyle(ui->textBrowser, style::properties::widget::border);
	ui->textBrowser->setStyleSheet(
		".QWidget {background-color: " + Style::getAttribute(json::theme::background_primary) + ";}");
	//	initReportButton();
}

ScopyHomeInfoPage::~ScopyHomeInfoPage() { delete ui; }

void ScopyHomeInfoPage::initReportButton()
{
	auto reportButton = new QPushButton("Report a bug");
	StyleHelper::BlueButton(reportButton, "reportButton");
	reportButton->setFixedSize(100, 40);

	auto reportBtnHoverWidget = new HoverWidget(reportButton, ui->textBrowser, this);
	reportBtnHoverWidget->setContentPos(HP_TOPLEFT);
	reportBtnHoverWidget->setAnchorPos(HP_BOTTOMRIGHT);
	reportBtnHoverWidget->setAnchorOffset(QPoint(-10, -10));
	reportBtnHoverWidget->setVisible(true);
	reportBtnHoverWidget->raise();

	connect(reportButton, &QPushButton::clicked, []() {
		const QUrl url("https://wiki.analog.com/university/tools/m2k/scopy/report");
		QDesktopServices::openUrl(url);
	});
}

#include "moc_scopyhomeinfopage.cpp"
