#include "pkgwidget.h"
#include "common/scopyconfig.h"

#include <QCompleter>
#include <QFileDialog>
#include <QJsonObject>
#include <application_restarter.h>
#include <installpkgdialog.h>
#include <pkginstaller.h>
#include <pkgitemwidget.h>
#include <pkgutil.h>
#include <qgridlayout.h>
#include <restartdialog.h>
#include <style.h>
#include <QSet>

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

	initZipSection(w);

	QWidget *pkgW = pkgSection(w);
	initPkgList();

	wLay->addWidget(m_zipSection, 0, Qt::AlignTop);
	wLay->addWidget(pkgW, 1, Qt::AlignTop);

	QWidget *restartW = createRestartWidget(this);
	restartW->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	lay->addWidget(w);
	lay->addWidget(restartW, Qt::AlignBottom);

	connect(PkgInstaller::GetInstance(), &PkgInstaller::zipMetadata, this, &PkgWidget::onZipMetadata);
	connect(PkgInstaller::GetInstance(), &PkgInstaller::pkgExists, this, &PkgWidget::showInstallDialog);
	connect(PkgInstaller::GetInstance(), &PkgInstaller::pkgInstalled, this,
		[this, restartW](bool restart) { restartW->setVisible(restart); });
	connect(PkgInstaller::GetInstance(), &PkgInstaller::pkgUninstalled, this,
		[this, restartW](bool restart) { restartW->setVisible(restart); });

	Style::setBackgroundColor(w, json::theme::background_subtle);
}

PkgWidget::~PkgWidget() {}

void PkgWidget::initZipSection(QWidget *parent)
{
	m_zipSection = new MenuSectionCollapseWidget("Inlcude ZIP packages", MenuCollapseSection::MHCW_NONE,
						     MenuCollapseSection::MHW_BASEWIDGET, parent);
	m_zipSection->contentLayout()->setSpacing(6);

	QWidget *browseWidget = new QWidget(m_zipSection);
	browseWidget->setLayout(new QHBoxLayout(browseWidget));
	browseWidget->layout()->setMargin(0);

	m_zipFileEdit = new MenuLineEdit(browseWidget);
	m_zipFileEdit->edit()->setPlaceholderText("Select ZIP");
	m_zipFileEdit->edit()->setReadOnly(true);
	QPushButton *browseBtn = new QPushButton("...", browseWidget);
	StyleHelper::BrowseButton(browseBtn);

	browseWidget->layout()->addWidget(m_zipFileEdit);
	browseWidget->layout()->addWidget(browseBtn);

	connect(browseBtn, &QPushButton::clicked, this, [this]() { browseFile(); });

	m_zipSection->add(new QLabel("Add a ZIP file containing the plugin you'd like to include"));
	m_zipSection->add(browseWidget);
}

QWidget *PkgWidget::pkgSection(QWidget *parent)
{
	MenuSectionCollapseWidget *pkgSection = new MenuSectionCollapseWidget(
		"Packages", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, parent);
	pkgSection->contentLayout()->setSpacing(6);
	pkgSection->collapseSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QStringList packages = PkgUtil::getPkgsName();

	QWidget *listW = pkgList(pkgSection);
	listW->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	pkgSection->add(listW);

	return pkgSection;
}

QWidget *PkgWidget::pkgList(QWidget *parent)
{
	QScrollArea *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);

	QWidget *wScroll = new QWidget(scroll);
	m_layScroll = new QVBoxLayout(wScroll);
	m_layScroll->setMargin(0);

	m_scrollSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layScroll->addItem(m_scrollSpacer);

	scroll->setWidget(wScroll);

	return scroll;
}

void PkgWidget::onZipMetadata(QVariantMap metadata)
{
	QWidget *w = new QWidget(this);
	QVBoxLayout *wLay = new QVBoxLayout(w);
	wLay->setMargin(0);

	QLineEdit *header = new QLineEdit("Package preview", w);
	header->setEnabled(false);
	header->setReadOnly(false);
	StyleHelper::MenuCollapseHeaderLineEdit(header, "zipPreviewHeader");

	PkgItemWidget *pkgItem = new PkgItemWidget(w);
	pkgItem->fillMetadata(metadata);
	pkgItem->setZipPath(m_zipFileEdit->edit()->text());

	wLay->addWidget(header);
	wLay->addWidget(pkgItem);

	m_zipSection->add(w);
	connect(pkgItem, &PkgItemWidget::installClicked, this, &PkgWidget::onInstall);
	connect(this, &PkgWidget::pkgReinstalled, pkgItem, &PkgItemWidget::setInstalled);
	connect(PkgInstaller::GetInstance(), &PkgInstaller::zipMetadata, w, &QWidget::deleteLater);
}

void PkgWidget::showInstallDialog(const QString &pkgName, const QString &zipPath)
{
	InstallPkgDialog *installDialog = new InstallPkgDialog(this);
	installDialog->setMessage("A package with the name " + pkgName +
				  " already exists. Are you sure you want to continue?");
	connect(installDialog, &InstallPkgDialog::yesClicked, this, [this, pkgName, zipPath, installDialog] {
		bool uninstalled = PkgInstaller::uninstall(pkgName, false);
		if(!uninstalled) {
			return;
		}
		bool installed = PkgInstaller::install(zipPath);
		if(installed) {
			Q_EMIT pkgReinstalled();
		}
		installDialog->deleteLater();
	});
	connect(installDialog, &InstallPkgDialog::noClicked, this, [installDialog] { installDialog->deleteLater(); });
	QMetaObject::invokeMethod(installDialog, &InstallPkgDialog::showDialog, Qt::QueuedConnection);
}

void PkgWidget::initPkgList()
{
	QJsonObject obj = PkgUtil::readLocalRepository(scopy::config::pkgLocalRepo());
	for(auto it = obj.begin(); it != obj.end(); ++it) {
		QJsonObject pkgMetadata = it.value().toObject();

		QWidget *container = new QWidget(this);
		container->setLayout(new QVBoxLayout(container));
		container->layout()->setContentsMargins(0, 10, 0, 10);
		PkgItemWidget *pkgItem = new PkgItemWidget(container);
		pkgItem->fillMetadata(pkgMetadata.toVariantMap(), true);
		container->layout()->addWidget(pkgItem);

		Style::setStyle(container, style::properties::widget::topBorder);
		addPkgItem(container);
		connect(pkgItem, &PkgItemWidget::uninstallClicked, this, &PkgWidget::onUninstall);
	}
}

void PkgWidget::browseFile()
{
	QString zipPath = QFileDialog::getOpenFileName(this, tr("Open"), "", "");
	m_zipFileEdit->edit()->setText(zipPath);
	PkgInstaller::preview(zipPath);
}

void PkgWidget::addPkgItem(QWidget *pkgItem)
{
	int spacerIdx = m_layScroll->indexOf(m_scrollSpacer);
	m_layScroll->insertWidget(spacerIdx, pkgItem);
}

void PkgWidget::onInstall()
{
	PkgItemWidget *pkgItem = dynamic_cast<PkgItemWidget *>(QObject::sender());
	if(!pkgItem || pkgItem->zipPath().isEmpty()) {
		return;
	}
	bool installed = PkgInstaller::install(pkgItem->zipPath());
	if(installed) {
		pkgItem->setInstalled();
	}
}

void PkgWidget::onUninstall()
{
	PkgItemWidget *pkgItem = dynamic_cast<PkgItemWidget *>(QObject::sender());
	if(!pkgItem) {
		return;
	}
	InstallPkgDialog *uninstallDialog = new InstallPkgDialog(this);
	uninstallDialog->setMessage("Are you sure you want to uninstall " + pkgItem->name() + "?");
	connect(uninstallDialog, &InstallPkgDialog::yesClicked, this, [this, uninstallDialog, pkgItem]() {
		bool uninstalled = PkgInstaller::uninstall(pkgItem->name());
		if(uninstalled) {
			pkgItem->setUninstalled();
		}
		uninstallDialog->deleteLater();
	});
	connect(uninstallDialog, &InstallPkgDialog::noClicked, [=] { uninstallDialog->deleteLater(); });
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
