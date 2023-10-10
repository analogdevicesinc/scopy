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

#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"
#include "src/externalpsreaderthread.h"
#include "src/swiotidentifytask.h"
#include "src/swiotpingtask.h"
#include "src/swiotreadtemperaturetask.h"
#include "src/swiotswitchctxtask.h"

#include <iio.h>

#include <QObject>

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

public Q_SLOTS:
	void identify();
	void writeModeAttribute(std::string mode);
	void readModeAttribute();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void contextSwitched();
	void hasConnectedPowerSupply(bool ps);
	void readTemperature(double temperature);
	void modeAttributeChanged(std::string mode);
	void isRuntimeCtxChanged(bool isRuntimeCtx);
private Q_SLOTS:
	void writeModeCommandFinished(scopy::Command *cmd);
	void readModeCommandFinished(scopy::Command *cmd);

private:
	void setIsRuntimeCtx(bool runtimeCtx);

	SwiotIdentifyTask *identifyTask;
	SwiotPingTask *pingTask;
	SwiotSwitchCtxTask *switchCtxTask;
	ExternalPsReaderThread *extPsTask;
	SwiotReadTemperatureTask *temperatureTask;
	CommandQueue *m_cmdQueue;
	iio_context *m_iioCtx;
	QString uri;
	bool m_isRuntimeCtx;
	bool m_temperatureReadEn;

	CyclicalTask *pingTimer;
	CyclicalTask *switchCtxTimer;
	CyclicalTask *powerSupplyTimer;
	CyclicalTask *temperatureTimer;
};
} // namespace scopy::swiot

#endif // SWIOTCONTROLLER_H
