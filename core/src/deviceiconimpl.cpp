#include "deviceiconimpl.h"

#include "gui/dynamicWidget.h"

#include "ui_devicebutton.h"

#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <style.h>

using namespace scopy;
DeviceIconImpl::DeviceIconImpl(Device *d, QWidget *parent)
	: DeviceIcon{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	ui->description->setText(d->displayParam());
	ui->name->setText(d->displayName());
	ui->iconPlaceHolder->layout()->addWidget(d->icon());
	ui->iconPlaceHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setStyleSheet("QWidget { background-color: transparent; }");
	Style::setStyle(this, style::properties::widget::deviceIcon, true, true);
	Style::setStyle(ui->line, style::properties::widget::deviceLine, false, true);

	setCheckable(true);
}

void DeviceIconImpl::setConnected(bool val)
{
	Style::setStyle(ui->line, style::properties::widget::deviceLine, val);
	//	ensurePolished();
}

DeviceIconImpl::~DeviceIconImpl() {}

#include "moc_deviceiconimpl.cpp"
