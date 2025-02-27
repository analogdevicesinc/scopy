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
