#include <scopy/gui/linked_button.hpp>
#include "dynamic_widget.hpp"

#include <QDesktopServices>
#include <QUrl>
#include <QFile>

using namespace scopy::gui;

LinkedButton::LinkedButton(QWidget* parent)
	: QPushButton(parent)
{
	DynamicWidget::setDynamicProperty(this,"blue_button", true);
	DynamicWidget::setDynamicProperty(this,"info_button", true);

	this->setToolTip("See more info");

	connect(this, SIGNAL(clicked()), this, SLOT(openUrl()));
}

void LinkedButton::openUrl() {
    QDesktopServices::openUrl(QUrl(this->m_url));
}

void LinkedButton::setUrl(QString url) {
	this->m_url = url;
}
