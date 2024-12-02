#include "qabstractbutton.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <license_overlay.h>

#include <pluginbase/preferences.h>

#include <QDesktopWidget>
#include <QFile>
#include <QPushButton>
#include <QStyle>
#include <QTextBrowser>
#include <stylehelper.h>
#include <ui_licensedialogoverlay.h>

using namespace scopy;

LicenseOverlay::LicenseOverlay(QWidget* parent):
	QWidget(parent),
	parent(parent)
{
	overlay = new gui::TintedOverlay(parent);
	ui = new Ui::LicenseDialogOverlay();

	ui->setupUi(this);
	ui->description->setText(getLicense());
	StyleHelper::BlueButton(ui->btnContinue, "btnContinue");
	StyleHelper::BlueButton(ui->btnExit, "btnExit");
	StyleHelper::OverlayMenu(this, "licenseOverlay");

	Preferences::connect(ui->btnContinue, &QAbstractButton::clicked, [&] () {
		Preferences::GetInstance()->set("general_first_run", false);
		overlay->deleteLater();
		deleteLater();
	});
	Preferences::connect(ui->btnExit, &QAbstractButton::clicked, [&] () {
		QCoreApplication::quit();
	});
}

void LicenseOverlay::showOverlay()
{
	overlay->show();
	raise();
	show();

	this->move(parent->rect().center() - this->rect().center());
}

QString LicenseOverlay::getLicense()
{
	QFile file(":/license.html");
	file.open(QIODevice::ReadOnly);
	QString text = QString(file.readAll());
	file.close();

	return text;
}
