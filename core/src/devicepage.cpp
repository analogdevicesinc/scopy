#include "devicepage.h"
#include "ui_devicepage.h"

DevicePage::DevicePage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DevicePage)
{
	ui->setupUi(this);
}

DevicePage::~DevicePage()
{
	delete ui;
}

#include "moc_devicepage.cpp"
