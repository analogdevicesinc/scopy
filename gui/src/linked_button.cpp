#include <scopy/gui/linked_button.hpp>
#include "dynamic_widget.hpp"

#include <QDesktopServices>
#include <QUrl>
#include <QFile>

using namespace scopy::gui;

LinkedButton::LinkedButton(QWidget* parent)
	: QPushButton(parent)
{
	DynamicWidget::setDynamicProperty(this,"info_button", true);

	// TODO: replace with this line
	//this->setIcon(QIcon::fromTheme("ico info"));

	// to be removed, used just for testing outside of scopy 2.0
	this->setIcon(QIcon(":/icons/ico info.png"));
	///

	this->setIconSize(QSize(32, 32));

	this->setToolTip("See more info");

	connect(this, SIGNAL(clicked()), this, SLOT(openUrl()));
}

void LinkedButton::openUrl() {
    QDesktopServices::openUrl(QUrl(this->m_url));
}

void LinkedButton::setUrl(QString url) {
	this->m_url = url;
}
