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

#include "pkginstalledtab.h"
#include "pkgmanifestfields.h"

#include <gui/widgets/menusectionwidget.h>
#include <installpkgdialog.h>
#include <menuonoffswitch.h>
#include <pkgmanager.h>
#include <pkgpreviewpage.h>
#include <style.h>

using namespace scopy;

PkgInstalledTab::PkgInstalledTab(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	QWidget *w = new QWidget(this);
	QVBoxLayout *wLay = new QVBoxLayout(w);
	wLay->setMargin(10);

	MenuSectionCollapseWidget *pkgSection = new MenuSectionCollapseWidget(
		"Installed packages", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, w);
	pkgSection->contentLayout()->setSpacing(10);
	pkgSection->collapseSection()->header()->setCheckable(false);
	pkgSection->menuSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QStringList searchOptions = PkgManager::getPkgsName();
	searchOptions.append(PkgManager::getPkgsCategory());
	searchOptions.append(PkgManager::getPkgsAuthor());
	searchOptions.append(PkgManager::getPkgsVersion());

	QWidget *searchW = new QWidget(pkgSection);
	QGridLayout *searchLay = new QGridLayout(searchW);
	searchLay->setMargin(0);

	m_searchBar = new SearchBar(QSet<QString>{searchOptions.begin(), searchOptions.end()}, searchW);
	m_searchBar->getLineEdit()->setPlaceholderText("Filter by keyword");
	m_searchBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	MenuOnOffSwitch *previewSwitch = new MenuOnOffSwitch("Preview", searchW);
	previewSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	previewSwitch->onOffswitch()->setChecked(true);

	searchLay->addWidget(m_searchBar, 0, 0);
	searchLay->addWidget(previewSwitch, 1, 0, Qt::AlignRight);

	m_pkgGrid = new PkgGridWidget(pkgSection);
	// The right margin is used to avoid the scroll bar overlapping with the packages.
	QMargins rightMargin(0, 0, 2, 0);
	m_pkgGrid->layout()->setContentsMargins(rightMargin);
	QScrollArea *scrollArea = new QScrollArea(pkgSection);
	scrollArea->setWidget(m_pkgGrid);
	scrollArea->setWidgetResizable(true);

	pkgSection->contentLayout()->addWidget(searchW);
	pkgSection->contentLayout()->addWidget(scrollArea);

	m_preview = new PkgPreviewPage(w);
	m_preview->hide();

	m_vSplitter = new QSplitter(Qt::Vertical, w);
	m_vSplitter->addWidget(pkgSection);
	m_vSplitter->addWidget(m_preview);

	wLay->addWidget(m_vSplitter);
	layout->addWidget(w);

	fillPkgSection();
	connect(m_searchBar->getLineEdit(), &QLineEdit::textChanged, this, [this](const QString &text) {
		QStringList searchParam = text.split(";", Qt::SkipEmptyParts);
		m_pkgGrid->searchPkg({PkgManifest::PKG_AUTHOR, PkgManifest::PKG_CATEGORY, PkgManifest::PKG_TITLE,
				      PkgManifest::PKG_VERSION},
				     searchParam);
	});
	connect(previewSwitch->onOffswitch(), &QAbstractButton::clicked, this, &PkgInstalledTab::previewSwitchClicked);

	Style::setBackgroundColor(w, json::theme::background_subtle);
}

PkgInstalledTab::~PkgInstalledTab() {}

PkgItemWidget *PkgInstalledTab::createPkgItemWidget(const QVariantMap &meta)
{
	PkgItemWidget *pkgItem = new PkgItemWidget(m_pkgGrid);
	pkgItem->fillMetadata(meta, true);
	connect(pkgItem, &PkgItemWidget::preview, this, &PkgInstalledTab::onPkgPreview);
	connect(pkgItem, &PkgItemWidget::hidePreview, this,
		[this]() { m_pkgGrid->updatePkgsStyle(m_preview->isEnabled()); });
	connect(pkgItem, &PkgItemWidget::categorySelected, this, &PkgInstalledTab::onCategorySelected);
	connect(pkgItem, &PkgItemWidget::hidePreview, m_preview, &PkgPreviewPage::hide);
	connect(pkgItem, &PkgItemWidget::uninstallClicked, this, &PkgInstalledTab::onUninstall);
	return pkgItem;
}

void PkgInstalledTab::fillPkgSection()
{
	const QList<QVariantMap> pkgsMeta = PkgManager::getPkgsMeta();
	for(const QVariantMap &meta : pkgsMeta) {
		PkgItemWidget *pkgItem = createPkgItemWidget(meta);
		pkgItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_pkgGrid->addPkg(pkgItem);
	}
}

void PkgInstalledTab::onUninstall()
{
	PkgItemWidget *pkgItem = dynamic_cast<PkgItemWidget *>(QObject::sender());
	if(!pkgItem) {
		return;
	}
	InstallPkgDialog *uninstallDialog = new InstallPkgDialog(this);
	uninstallDialog->setMessage("Are you sure you want to uninstall " + pkgItem->id() + "?");
	connect(uninstallDialog, &InstallPkgDialog::yesClicked, this, [this, uninstallDialog, pkgItem]() {
		bool uninstalled = PkgManager::uninstall(pkgItem->id());
		pkgItem->uninstallFinished(uninstalled);
		uninstallDialog->deleteLater();
	});
	connect(uninstallDialog, &InstallPkgDialog::noClicked, this, [this, uninstallDialog, pkgItem]() {
		pkgItem->uninstallFinished(false);
		uninstallDialog->deleteLater();
	});
	QMetaObject::invokeMethod(uninstallDialog, &InstallPkgDialog::showDialog, Qt::QueuedConnection);
}

void PkgInstalledTab::previewSwitchClicked(bool checked)
{
	m_preview->setEnabled(checked);
	if(!checked) {
		m_preview->setVisible(checked);
	}
	m_pkgGrid->updatePkgsStyle(checked);
}

void PkgInstalledTab::onPkgPreview(const QVariantMap &metadata)
{
	bool previewEnabled = m_preview->isEnabled();
	if(previewEnabled) {
		m_preview->updatePreview(metadata);
		m_preview->show();
		m_pkgGrid->updatePkgsStyle(previewEnabled);
	}
}

void PkgInstalledTab::onCategorySelected(const QString &category)
{
	QLineEdit *lineEdit = m_searchBar->getLineEdit();
	QStringList searchingList = lineEdit->text().split(";", Qt::SkipEmptyParts);
	if(searchingList.contains(category)) {
		searchingList.removeAll(category);
	} else {
		searchingList.append(category);
	}
	lineEdit->setText(searchingList.join(";"));
}

#include "moc_pkginstalledtab.cpp"
