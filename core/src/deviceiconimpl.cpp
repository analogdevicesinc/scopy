#include "deviceiconimpl.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include "gui/dynamicWidget.h"
#include "stylehelper.h"
#include "ui_devicebutton.h"

using namespace scopy;
DeviceIconImpl::DeviceIconImpl(Device *d, QWidget *parent)
	: DeviceIcon{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	StyleHelper::DeviceIconBackgroundShadow(this);
	ui->description->setText(d->displayParam());
	ui->name->setText(d->displayName());
	ui->iconPlaceHolder->layout()->addWidget(d->icon());
	ui->iconPlaceHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );

	setCheckable(true);
}

void DeviceIconImpl::setConnected(bool val) {
	setDynamicProperty(ui->line, "connected", val);
	//	ensurePolished();
}

DeviceIconImpl::~DeviceIconImpl() {}

#include "moc_deviceiconimpl.cpp"
