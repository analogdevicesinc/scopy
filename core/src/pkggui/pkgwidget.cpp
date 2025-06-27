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

#include "pkgwidget.h"
#include <QCompleter>
#include <QFileDialog>
#include <QJsonObject>
#include <application_restarter.h>
#include <installpkgdialog.h>
#include <pkgcard.h>
#include <qgridlayout.h>
#include <restartdialog.h>
#include <style.h>
#include <QSet>
#include <pkg-manager/pkgmanager.h>
#include <pkgzipwidget.h>
#include <pkggridwidget.h>
#include <pkginstalledtab.h>

using namespace scopy;

PkgWidget::PkgWidget(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	m_tabWidget = new VerticalTabWidget(this);

	PkgZipWidget *zipWidget = new PkgZipWidget(m_tabWidget);
	zipWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	PkgInstalledTab *installedTab = new PkgInstalledTab(m_tabWidget);

	m_tabWidget->addTab(installedTab, "Installed");
	m_tabWidget->addTab(zipWidget, "Install from file");

	QWidget *restartW = createRestartWidget(this);
	restartW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	layout->addWidget(m_tabWidget);
	layout->addWidget(restartW, Qt::AlignBottom);

	PkgManager *pkgManager = PkgManager::GetInstance();
	connect(zipWidget, &PkgZipWidget::pkgPathSelected, pkgManager, &PkgManager::preview);
	connect(zipWidget, &PkgZipWidget::installClicked, this, &PkgWidget::onInstall);
	connect(pkgManager, &PkgManager::zipMetadata, zipWidget, &PkgZipWidget::onZipMetadata);

	connect(pkgManager, &PkgManager::pkgExists, this, &PkgWidget::showInstallDialog);
	connect(pkgManager, &PkgManager::pkgInstalled, this,
		[this, restartW](bool restart) { restartW->setVisible(restart); });
	connect(pkgManager, &PkgManager::pkgUninstalled, this,
		[this, restartW](bool restart) { restartW->setVisible(restart); });

	// Must be deleted. Another method needs to be found to ensure that the event was triggered from zipWidget
	// and not from somewhere else.
	connect(pkgManager, &PkgManager::pkgInstalled, zipWidget, &PkgZipWidget::onInstallFinished);
}

PkgWidget::~PkgWidget() {}

QWidget *PkgWidget::pkgList(QWidget *parent)
{
	QScrollArea *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setMinimumHeight(300);

	QWidget *wScroll = new QWidget(scroll);
	m_layScroll = new QVBoxLayout(wScroll);
	m_layScroll->setMargin(0);
	m_layScroll->setSpacing(10);

	m_scrollSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layScroll->addItem(m_scrollSpacer);

	scroll->setWidget(wScroll);

	return scroll;
}

void PkgWidget::showInstallDialog(const QString &zipPath, const QString &pkgName)
{
	InstallPkgDialog *installDialog = new InstallPkgDialog(this);
	installDialog->setMessage("A package with the name " + pkgName +
				  " already exists. Are you sure you want to continue?");
	connect(installDialog, &InstallPkgDialog::yesClicked, this, [this, pkgName, zipPath, installDialog] {
		bool uninstalled = PkgManager::uninstall(pkgName, false);
		if(!uninstalled) {
			return;
		}
		bool installed = PkgManager::install(zipPath);
		Q_EMIT pkgReinstalled(installed);
		installDialog->deleteLater();
	});
	connect(installDialog, &InstallPkgDialog::noClicked, this, [installDialog] { installDialog->deleteLater(); });
	QMetaObject::invokeMethod(installDialog, &InstallPkgDialog::showDialog, Qt::QueuedConnection);
}

void PkgWidget::onInstall(const QString &zipPath) { PkgManager::install(zipPath); }

QWidget *PkgWidget::createRestartWidget(QWidget *parent)
{
	QWidget *restartWidget = new QWidget(parent);
	QHBoxLayout *lay = new QHBoxLayout(restartWidget);
	lay->setSpacing(0);
	lay->setMargin(10);
	restartWidget->setVisible(false);
	QLabel *lab = new QLabel("An application restart is required for these changes to take effect. ");
	QSpacerItem *space1 = new QSpacerItem(6, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QSpacerItem *space2 = new QSpacerItem(6, 20, QSizePolicy::Preferred, QSizePolicy::Fixed);
	QPushButton *btn = new QPushButton("Restart");
	Style::setStyle(btn, style::properties::button::basicButton, true, true);
	StyleHelper::BackgroundWidget(restartWidget, "restartWidget");
	btn->setFixedWidth(100);

	lay->addWidget(btn);
	lay->addSpacerItem(space2);
	lay->addWidget(lab);
	lay->addSpacerItem(space1);

	connect(btn, &QPushButton::clicked, btn, []() { ApplicationRestarter::triggerRestart(); });

	return restartWidget;
}

#include "moc_pkgwidget.cpp"
