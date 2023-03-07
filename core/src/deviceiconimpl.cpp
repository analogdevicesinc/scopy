#include "deviceiconimpl.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include "gui/dynamicWidget.h"
#include "ui_devicebutton.h"

using namespace adiscope;
DeviceIconImpl::DeviceIconImpl(Device *d, QWidget *parent)
	: DeviceIcon{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	ui->description->setText(d->description());
	ui->name->setText(d->name());
	ui->iconPlaceHolder->layout()->addWidget(d->icon());
	ui->iconPlaceHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
	QPushButton *forgetBtn = new QPushButton("X",d->icon());
	forgetBtn->setStyleSheet("color:red");
	QLabel *connectionBadge = new QLabel("",d->icon());
	forgetBtn->setMaximumSize(20,20);
	forgetBtn->move(80,0);
	connectionBadge->setMaximumSize(20,20);
	connectionBadge->move(0,0);
	connectionBadge->setText(QString(d->description()[0]));
//	forgetBtn->move(30,30);
	forgetBtn->raise();
	connect(forgetBtn,SIGNAL(clicked()),this,SIGNAL(forget()));
	setCheckable(true);
}

void DeviceIconImpl::setConnected(bool val) {
	setDynamicProperty(ui->line, "connected", val);
//	ensurePolished();
}



DeviceIconImpl::~DeviceIconImpl() {}
void DeviceIconImpl::paintEvent(QPaintEvent *e) {
	QWidget::paintEvent(e);
}

#include "moc_deviceiconimpl.cpp"
