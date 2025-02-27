/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "installpkgdialog.h"

#include <style.h>

using namespace scopy;

InstallPkgDialog::InstallPkgDialog(QWidget *parent)
	: QWidget(parent)
	, parent(parent)
{
	m_popupWidget = new PopupWidget(parent);
	m_popupWidget->setFocusOnContinueButton();
	m_popupWidget->setEnableExternalLinks(true);
	m_popupWidget->enableTitleBar(false);
	m_popupWidget->enableTintedOverlay(true);
	m_popupWidget->enableCenterOnParent(true);
	m_popupWidget->getContinueBtn()->setText("Yes");
	m_popupWidget->getExitBtn()->setText("No");

	connect(m_popupWidget->getContinueBtn(), &QAbstractButton::clicked, this, &InstallPkgDialog::yesClicked);
	connect(m_popupWidget->getExitBtn(), &QAbstractButton::clicked, this, &InstallPkgDialog::noClicked);

	Style::setBackgroundColor(m_popupWidget, json::theme::background_primary);
}

InstallPkgDialog::~InstallPkgDialog() { m_popupWidget->deleteLater(); }

void InstallPkgDialog::showDialog()
{
	raise();
	show();
}

void InstallPkgDialog::setMessage(const QString &msg) { m_popupWidget->setDescription(msg); }

#include "moc_installpkgdialog.cpp"
