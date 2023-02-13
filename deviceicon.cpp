#include "deviceicon.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include "gui/dynamicWidget.h"
#include "ui_devicebutton.h"

using namespace adiscope;
DeviceIcon::DeviceIcon(QString name, QString description, QWidget *icon, QWidget *parent)
	: QAbstractButton{parent}
{
	ui = new Ui::DeviceButton;
	ui->setupUi(this);
	ui->description->setText(description);
	ui->name->setText(name);
	ui->iconPlaceHolder->layout()->addWidget(icon);
	setCheckable(true);
}

void DeviceIcon::setConnected(bool val) {
	setDynamicProperty(ui->line, "connected", val);
}



DeviceIcon::~DeviceIcon() {}
void DeviceIcon::paintEvent(QPaintEvent *e) {
	QWidget::paintEvent(e);
}
