#include "linked_button.hpp"

#include "dynamicWidget.h"

#include <QDesktopServices>
#include <QFile>
#include <QUrl>
#include <style.h>

using namespace scopy;

LinkedButton::LinkedButton(QWidget *parent)
	: QPushButton(parent)
{
	setDynamicProperty(this, "info_button", true);

	this->setIcon(
		Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/info"));
	this->setIconSize(QSize(32, 32));

	this->setToolTip("See more info");

	connect(this, SIGNAL(clicked()), this, SLOT(openUrl()));
}

void LinkedButton::openUrl() { QDesktopServices::openUrl(QUrl(this->url)); }

void LinkedButton::setUrl(QString url) { this->url = url; }

#include "moc_linked_button.cpp"
