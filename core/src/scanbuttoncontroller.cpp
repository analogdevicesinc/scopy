#include "scanbuttoncontroller.h"

using namespace scopy;

ScanButtonController::ScanButtonController(CyclicalTask *cs, QPushButton *btn, QObject *parent)
	: QObject{parent}
{
	this->cs = cs;
	this->btn = btn;
	m_scanTimeout = 2000;
	conn = connect(this->btn, SIGNAL(toggled(bool)), this, SLOT(enableScan(bool)));
}
ScanButtonController::~ScanButtonController() { disconnect(conn); }

void ScanButtonController::enableScan(bool b)
{
	if(b) {
		startScan();
	} else {
		stopScan();
	}
}

void ScanButtonController::startScan()
{
	cs->start(m_scanTimeout);
	btn->setChecked(true);
}

void ScanButtonController::stopScan()
{
	cs->stop();
	btn->setChecked(false);
}

int ScanButtonController::scanTimeout() const { return m_scanTimeout; }

void ScanButtonController::setScanTimeout(int newScanTimeout) { m_scanTimeout = newScanTimeout; }

#include "moc_scanbuttoncontroller.cpp"
