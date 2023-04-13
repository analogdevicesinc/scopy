#include "scanbuttoncontroller.h"

using namespace scopy;

ScanButtonController::ScanButtonController(CyclicalTask *cs , QPushButton *btn, QObject *parent)
	: QObject{parent}
{
	this->cs = cs;
	this->btn = btn;
	conn = connect(this->btn,SIGNAL(toggled(bool)),this,SLOT(enableScan(bool)));
}
ScanButtonController::~ScanButtonController() {
	disconnect(conn);
}

void ScanButtonController::enableScan(bool b) {
	if(b)
		startScan();
	else
		stopScan();
}

void ScanButtonController::startScan() {
	cs->start(2000);
	btn->setChecked(true);
}

void ScanButtonController::stopScan() {
	cs->stop();
	btn->setChecked(false);
}

#include "moc_scanbuttoncontroller.cpp"
