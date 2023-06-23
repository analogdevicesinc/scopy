/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */


#ifndef SWIOTCONTROLLER_H
#define SWIOTCONTROLLER_H

#include <QObject>
#include <iio.h>
#include "iioutil/iiopingtask.h"
#include "iioutil/cyclicaltask.h"
#include "src/externalpsreaderthread.h"
#include "src/swiotswitchctxtask.h"
#include "src/swiotreadtemperaturetask.h"


namespace scopy::swiot {
class SwiotController : public QObject
{
	Q_OBJECT
public:
	SwiotController(QString uri, QObject *parent = nullptr);
	~SwiotController();

	void startPingTask();
	void stopPingTask();

	void startPowerSupplyTask(QString attribute);
	void stopPowerSupplyTask();

	void startSwitchContextTask(bool isRuntime);
	void stopSwitchContextTask();

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectSwiot(iio_context *ctx);
	void disconnectSwiot();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void contextSwitched();
	void hasConnectedPowerSupply(bool ps);
	void readTemperature(double temperature);

private:
	IIOPingTask *pingTask;
	SwiotSwitchCtxTask *switchCtxTask;
	ExternalPsReaderThread *extPsTask;
	SwiotReadTemperatureTask *temperatureTask;
	iio_context *m_iioCtx;
	QString uri;

	CyclicalTask *pingTimer;
	CyclicalTask *switchCtxTimer;
	CyclicalTask *powerSupplyTimer;
	CyclicalTask *temperatureTimer;
};
}


#endif // SWIOTCONTROLLER_H
