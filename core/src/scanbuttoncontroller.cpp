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

using namespace scopy;

ScanButtonController::ScanButtonController(CyclicalTask *cs, QPushButton *btn, QObject *parent)
	: QObject{parent}
{
	this->cs = cs;
	this->btn = btn;
	conn = connect(this->btn, SIGNAL(toggled(bool)), this, SLOT(enableScan(bool)));
}
ScanButtonController::~ScanButtonController() { disconnect(conn); }

void ScanButtonController::enableScan(bool b)
{
	if(b)
		startScan();
	else
		stopScan();
}

void ScanButtonController::startScan()
{
	cs->start(2000);
	btn->setChecked(true);
}

void ScanButtonController::stopScan()
{
	cs->stop();
	btn->setChecked(false);
}

#include "moc_scanbuttoncontroller.cpp"
