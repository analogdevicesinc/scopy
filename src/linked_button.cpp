#include "linked_button.hpp"
#include "dynamicWidget.hpp"

#include <QDesktopServices>
#include <QUrl>
#include <QFile>

using namespace adiscope;

LinkedButton::LinkedButton(QWidget* parent)
	: QPushButton(parent)
{
	setDynamicProperty(this,"info_button", true);

	this->setIcon(QIcon::fromTheme("ico info"));
	this->setIconSize(QSize(32, 32));

	this->setToolTip("See more info");

	connect(this, SIGNAL(clicked()), this, SLOT(openUrl()));
}

void LinkedButton::openUrl() {
    QDesktopServices::openUrl(QUrl(this->url));
}

void LinkedButton::setUrl(QString url) {
	this->url = url;
}
