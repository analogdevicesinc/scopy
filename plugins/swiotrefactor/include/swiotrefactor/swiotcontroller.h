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
#include "externalpsreaderthread.h"
#include "swiotidentifytask.h"
#include "swiotpingtask.h"
#include "swiotreadtemperaturetask.h"

#include <iio.h>

#include <QObject>

namespace scopy::swiotrefactor {
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

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectSwiot();
	void disconnectSwiot();

public Q_SLOTS:
	void identify();
	void writeModeAttribute(QString mode);
	void readModeAttribute();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void hasConnectedPowerSupply(bool ps);
	void readTemperature(double temperature);
	void modeAttributeChanged(QString mode);
	void isRuntimeCtxChanged(bool isRuntimeCtx);
	void writeModeFailed();
private Q_SLOTS:
	void writeModeCommandFinished(scopy::Command *cmd);
	void readModeCommandFinished(scopy::Command *cmd);

private:
	void setIsRuntimeCtx(bool runtimeCtx);

	SwiotIdentifyTask *identifyTask;
	SwiotPingTask *pingTask;
	ExternalPsReaderThread *extPsTask;
	SwiotReadTemperatureTask *temperatureTask;
	CommandQueue *m_cmdQueue;
	iio_context *m_iioCtx;
	Connection *m_conn;
	QString uri;
	bool m_isRuntimeCtx;
	bool m_temperatureReadEn;

	CyclicalTask *pingTimer;
	CyclicalTask *powerSupplyTimer;
	CyclicalTask *temperatureTimer;
};
} // namespace scopy::swiotrefactor

#endif // SWIOTCONTROLLER_H
