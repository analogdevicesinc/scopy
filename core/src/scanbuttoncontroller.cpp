/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scanbuttoncontroller.h"

#include "devicescanner.h"

using namespace scopy;

ScanButtonController::ScanButtonController(DeviceScanner *scanner, QCheckBox *btn, QObject *parent)
	: QObject{parent}
{
	m_scanner = scanner;
	this->btn = btn;
	m_scanTimeout = 2000;
	conn = connect(this->btn, &QAbstractButton::toggled, this, &ScanButtonController::enableScan);
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
	m_scanner->start(m_scanTimeout);
	btn->setChecked(true);
}

void ScanButtonController::stopScan()
{
	m_scanner->stop();
	btn->setChecked(false);
}

int ScanButtonController::scanTimeout() const { return m_scanTimeout; }

void ScanButtonController::setScanTimeout(int newScanTimeout) { m_scanTimeout = newScanTimeout; }

#include "moc_scanbuttoncontroller.cpp"
