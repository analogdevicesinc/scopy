#include <QAbstractButton>
#include <QCoreApplication>

#include <license_overlay.h>
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

#include "moc_license_overlay.cpp"
