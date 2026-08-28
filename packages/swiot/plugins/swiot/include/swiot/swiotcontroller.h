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

#include "externalpsreader.h"
#include "swiotidentifytask.h"
#include "swiotreadtemperaturetask.h"

#include <QObject>
#include <QTimer>
#include <qcoro/qcorotask.h>
#include <component/controller.h>

namespace scopy {
namespace component {
class Device;
}
namespace swiot {
class SwiotController : public QObject
{
	Q_OBJECT
public:
	SwiotController(QString uri, QObject *parent = nullptr);
	~SwiotController();

	void startPowerSupplyTask(QString attribute);
	void stopPowerSupplyTask();

	void startTemperatureTask();
	void stopTemperatureTask();

	void connectSwiot();
	void disconnectSwiot();

	QCoro::Task<void> writeModeAttribute(QString mode);
	QCoro::Task<void> readModeAttribute();

public Q_SLOTS:
	void identify();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
	void hasConnectedPowerSupply(bool ps);
	void readTemperature(double temperature);
	void modeAttributeChanged(QString mode);
	void isRuntimeCtxChanged(bool isRuntimeCtx);
	void writeModeFailed();

private:
	void setIsRuntimeCtx(bool runtimeCtx);

	SwiotIdentifyTask *m_identifyTask;
	ExternalPsReader *m_extPsTask;
	SwiotReadTemperatureTask *m_temperatureTask;

	component::ContextHandle m_context;
	component::Device *m_swiotDevice;

	QString uri;
	bool m_isRuntimeCtx;
	bool m_temperatureReadEn;

	QTimer *powerSupplyTimer;
	QTimer *temperatureTimer;
};
} // namespace swiot
} // namespace scopy

#endif // SWIOTCONTROLLER_H
