#include "versionoverlay.h"
#include <pluginbase/preferences.h>
//#include "ui_popupwidget.h"


using namespace scopy;
VersionCheckOverlay::VersionCheckOverlay(QWidget *parent) : PopupWidget(parent) {
	setDescription("Do you want to automatically check for newer Scopy and m2k-firmware versions?\n\nYou can change this anytime from the Preferences menu.");
	setExitButtonText("No");
	setContinueButtonText("Yes");
	enableTitleBar(false);
	enableTintedOverlay(true);

	connect(this, &PopupWidget::continueButtonClicked, this, [this] () {
		scopy::Preferences::set("general_check_online_version", true);
		delete this;
	});
	connect(this, &PopupWidget::exitButtonClicked, this, [this] () {
		scopy::Preferences::set("general_check_online_version", false);
		delete this;
	});
}

VersionCheckOverlay::~CheckUpdateOverlay() {

}

void VersionCheckOverlay::showOverlay() {
	raise();
	show();

	this->move(parentWidget()->rect().center() - this->rect().center());
}

QPushButton VersionCheckOverlay::getExitBtn()
{
	return ui->continueButton;
}

#include "moc_versioncheckoverlay.cpp"
