#include "qabstractbutton.h"

#include "restartdialog.h"
using namespace scopy::gui;

RestartDialog::RestartDialog(QWidget *parent)
	: QDialog(parent)
	, parent(parent)
{
	m_popupWidget = new PopupWidget(this);
	m_popupWidget->setFocusOnContinueButton();
	m_popupWidget->enableTitleBar(false);
	m_popupWidget->setEnableExternalLinks(true);
	m_popupWidget->getExitBtn()->hide();
	m_popupWidget->getContinueBtn()->setText("Restart");
	connect(m_popupWidget, &PopupWidget::continueButtonClicked, [&]() { Q_EMIT restartButtonClicked(); });
}

RestartDialog::~RestartDialog() { delete m_popupWidget; }

void RestartDialog::setDescription(const QString &description) { m_popupWidget->setDescription(description); }

void RestartDialog::showDialog()
{
	raise();
	show();
}

#include "moc_restartdialog.cpp"
