#include "pkgwidget.h"
#include <QCompleter>
#include <QFileDialog>
#include <QJsonObject>
#include <application_restarter.h>
#include <installpkgdialog.h>
#include <pkgitemwidget.h>
#include <qgridlayout.h>
#include <restartdialog.h>
#include <style.h>
#include <QSet>
#include <pkgmanager.h>
#include <pkgzipwidget.h>
#include <pkggridwidget.h>

using namespace scopy;

PkgWidget::PkgWidget(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);

	QWidget *w = new QWidget(this);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *wLay = new QVBoxLayout(w);
	wLay->setMargin(10);

	PkgZipWidget *zipWidget = new PkgZipWidget(w);

	QWidget *pkgW = pkgSection(w);
	fillPkgSection(pkgW);

	wLay->addWidget(zipWidget, 0, Qt::AlignTop);
	wLay->addWidget(pkgW, 1, Qt::AlignTop);

	QWidget *restartW = createRestartWidget(this);
	restartW->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	lay->addWidget(w);
	lay->addWidget(restartW, Qt::AlignBottom);

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

	Style::setBackgroundColor(w, json::theme::background_subtle);
}

PkgWidget::~PkgWidget() {}

void PkgWidget::fillPkgSection(QWidget *parent)
{
	const QList<QVariantMap> pkgsMeta = PkgManager::getPkgsMeta();
	for(const QVariantMap &meta : pkgsMeta) {
		PkgItemWidget *pkgItem = new PkgItemWidget(parent);
		pkgItem->fillMetadata(meta, true);
		pkgItem->setSingleVersion(true);
		Style::setStyle(pkgItem, style::properties::widget::border_interactive);
		m_pkgGrid->addPkg(pkgItem);
		connect(pkgItem, &PkgItemWidget::uninstallClicked, this, &PkgWidget::onUninstall);
	}
}

QWidget *PkgWidget::pkgSection(QWidget *parent)
{
	QWidget *container = new QWidget(parent);
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	container->setLayout(new QVBoxLayout(container));
	container->layout()->setMargin(10);
	Style::setStyle(container, style::properties::widget::border_interactive);
	Style::setBackgroundColor(container, json::theme::background_primary);

	MenuSectionCollapseWidget *pkgSection = new MenuSectionCollapseWidget(
		"Packages", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, container);
	pkgSection->contentLayout()->setSpacing(6);
	pkgSection->menuSection()->contentLayout()->setMargin(0);
	pkgSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QStringList packages = PkgManager::getInstalledPkgs();
	m_searchBar = new SearchBar(QSet(packages.begin(), packages.end()), pkgSection);

	m_pkgGrid = new PkgGridWidget(pkgSection);
	// The right margin is used to avoid the scroll bar overlapping with the packages.
	QMargins rightMargin(0, 0, 2, 0);
	m_pkgGrid->layout()->setContentsMargins(rightMargin);
	QScrollArea *scrollArea = new QScrollArea(pkgSection);
	scrollArea->setWidget(m_pkgGrid);
	scrollArea->setWidgetResizable(true);

	pkgSection->add(m_searchBar);
	pkgSection->add(scrollArea);

	container->layout()->addWidget(pkgSection);

	connect(m_searchBar->getLineEdit(), &QLineEdit::textChanged, m_pkgGrid, &PkgGridWidget::searchPkg);

	return container;
}

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

void PkgWidget::onUninstall()
{
	PkgItemWidget *pkgItem = dynamic_cast<PkgItemWidget *>(QObject::sender());
	if(!pkgItem) {
		return;
	}
	InstallPkgDialog *uninstallDialog = new InstallPkgDialog(this);
	uninstallDialog->setMessage("Are you sure you want to uninstall " + pkgItem->name() + "?");
	connect(uninstallDialog, &InstallPkgDialog::yesClicked, this, [this, uninstallDialog, pkgItem]() {
		bool uninstalled = PkgManager::uninstall(pkgItem->name());
		pkgItem->uninstallFinished(uninstalled);
		uninstallDialog->deleteLater();
	});
	connect(uninstallDialog, &InstallPkgDialog::noClicked, this, [this, uninstallDialog, pkgItem]() {
		pkgItem->uninstallFinished(false);
		uninstallDialog->deleteLater();
	});
	QMetaObject::invokeMethod(uninstallDialog, &InstallPkgDialog::showDialog, Qt::QueuedConnection);
}

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
