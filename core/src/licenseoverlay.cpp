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

#include <QAbstractButton>
#include <QCoreApplication>

#include <licenseoverlay.h>
#include <pluginbase/preferences.h>
using namespace scopy;

LicenseOverlay::LicenseOverlay(QWidget *parent)
	: QWidget(parent)
	, parent(parent)
{
	m_popupWidget = new PopupWidget(parent);
	m_popupWidget->setFocusOnContinueButton();
	m_popupWidget->setEnableExternalLinks(true);
	m_popupWidget->enableTitleBar(false);
	m_popupWidget->enableTintedOverlay(true);
	m_popupWidget->setDescription(getLicense());

	connect(m_popupWidget->getContinueBtn(), &QAbstractButton::clicked, [&]() {
		Preferences::GetInstance()->set("general_first_run", false);
		deleteLater();
	});
	Preferences::connect(m_popupWidget->getExitBtn(), &QAbstractButton::clicked,
			     [&]() { QCoreApplication::quit(); });
}

LicenseOverlay::~LicenseOverlay() { delete m_popupWidget; }

void LicenseOverlay::showOverlay()
{
	raise();
	show();
	m_popupWidget->move(parent->rect().center() - m_popupWidget->rect().center());
}

QPushButton *LicenseOverlay::getContinueBtn() { return m_popupWidget->getContinueBtn(); }

QString LicenseOverlay::getLicense()
{
	QFile file(":/license.html");
	file.open(QIODevice::ReadOnly);
	QString text = QString(file.readAll());
	file.close();

	return text;
}

#include "moc_licenseoverlay.cpp"
