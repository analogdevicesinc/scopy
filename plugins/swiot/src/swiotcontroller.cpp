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

#include "swiotcontroller.h"

#include "src/swiot_logging_categories.h"

#include <iioutil/commandqueueprovider.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributewrite.h>

using namespace scopy;
using namespace scopy::swiot;

SwiotController::SwiotController(QString uri, QObject *parent)
	: QObject(parent)
	, uri(uri)
	, m_isRuntimeCtx(false)
	, m_temperatureReadEn(false)
{
	pingTask = nullptr;
	pingTimer = nullptr;
	temperatureTask = nullptr;
	temperatureTimer = nullptr;
	extPsTask = nullptr;
	powerSupplyTimer = nullptr;
	identifyTask = nullptr;
	m_cmdQueue = nullptr;
}

SwiotController::~SwiotController() {}

void SwiotController::startPingTask()
{
	pingTask = new SwiotPingTask(m_iioCtx);
	pingTimer = new CyclicalTask(pingTask);
	connect(pingTask, SIGNAL(pingSuccess()), this, SIGNAL(pingSuccess()));
	connect(pingTask, SIGNAL(pingFailed()), this, SIGNAL(pingFailed()));
	pingTimer->start(2000);
}

void SwiotController::stopPingTask()
{
	pingTask->requestInterruption();
	pingTimer->deleteLater();
	pingTask->deleteLater();
}

void SwiotController::startSwitchContextTask(bool isRuntime)
{
	switchCtxTask = new SwiotSwitchCtxTask(uri, isRuntime);
	switchCtxTimer = new CyclicalTask(switchCtxTask, this);
	connect(switchCtxTask, &SwiotSwitchCtxTask::contextSwitched, this, &SwiotController::contextSwitched);
	switchCtxTimer->start(1000);
}

void SwiotController::stopSwitchContextTask()
{
	switchCtxTimer->stop();
	switchCtxTask->requestInterruption();
	disconnect(switchCtxTask, &SwiotSwitchCtxTask::contextSwitched, this, &SwiotController::contextSwitched);
}

void SwiotController::connectSwiot(iio_context *ctx)
{
	m_iioCtx = ctx;
	if(m_iioCtx) {
		m_cmdQueue = CommandQueueProvider::GetInstance()->open(m_iioCtx);
		readModeAttribute();
	}
}

void SwiotController::disconnectSwiot()
{
	if(m_cmdQueue) {
		CommandQueueProvider::GetInstance()->close(m_iioCtx);
		m_cmdQueue = nullptr;
	}
	m_iioCtx = nullptr;
}

void SwiotController::startPowerSupplyTask(QString attribute)
{
	extPsTask = new ExternalPsReaderThread(uri, attribute, this);
	powerSupplyTimer = new CyclicalTask(extPsTask);
	connect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this,
		&SwiotController::hasConnectedPowerSupply);
	powerSupplyTimer->start(5000);
}

void SwiotController::stopPowerSupplyTask()
{
	powerSupplyTimer->stop();
	extPsTask->requestInterruption();
	disconnect(extPsTask, &ExternalPsReaderThread::hasConnectedPowerSupply, this,
		   &SwiotController::hasConnectedPowerSupply);
	powerSupplyTimer->deleteLater();
	extPsTask->deleteLater();
}

void SwiotController::startTemperatureTask()
{
	if(!m_isRuntimeCtx || m_temperatureReadEn) {
		return;
	}
	temperatureTask = new SwiotReadTemperatureTask(uri, this);
	temperatureTimer = new CyclicalTask(temperatureTask);
	connect(temperatureTask, &SwiotReadTemperatureTask::newTemperature, this, &SwiotController::readTemperature);
	temperatureTimer->start(2000);
	m_temperatureReadEn = true;
}

void SwiotController::stopTemperatureTask()
{
	if(!m_isRuntimeCtx || !m_temperatureReadEn) {
		return;
	}
	temperatureTask->requestInterruption();
	temperatureTimer->stop();
	disconnect(temperatureTask, &SwiotReadTemperatureTask::newTemperature, this, &SwiotController::readTemperature);
	temperatureTimer->deleteLater();
	temperatureTask->deleteLater();
	m_temperatureReadEn = false;
}

void SwiotController::identify()
{
	if(!identifyTask) {
		identifyTask = new SwiotIdentifyTask(uri);
		identifyTask->start();
		connect(identifyTask, &QThread::finished, this, [=, this]() {
			delete identifyTask;
			identifyTask = nullptr;
		});
	}
}

void SwiotController::writeModeAttribute(std::string mode)
{
	if(!m_iioCtx || !m_cmdQueue) {
		return;
	}
	struct iio_device *swiot = iio_context_find_device(m_iioCtx, "swiot");
	if(swiot) {
		Command *writeModeCommand = new IioDeviceAttributeWrite(swiot, "mode", mode.c_str(), nullptr);
		connect(writeModeCommand, &scopy::Command::finished, this, &SwiotController::writeModeCommandFinished,
			Qt::QueuedConnection);
		m_cmdQueue->enqueue(writeModeCommand);
	} else {
		qDebug(CAT_SWIOT) << "Can't find swiot iio_device";
	}
}

void SwiotController::writeModeCommandFinished(scopy::Command *cmd)
{
	IioDeviceAttributeWrite *tcmd = dynamic_cast<IioDeviceAttributeWrite *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		Q_EMIT modeAttributeChanged(tcmd->getAttributeValue());
		qInfo(CAT_SWIOT) << R"(Successfully changed the swiot mode)";
	} else {
		qDebug(CAT_SWIOT) << R"(Error, could not change swiot mode)" << tcmd->getReturnCode();
	}
}

void SwiotController::readModeAttribute()
{
	if(!m_iioCtx || !m_cmdQueue) {
		return;
	}
	struct iio_device *swiot = iio_context_find_device(m_iioCtx, "swiot");
	if(swiot) {
		Command *readModeCommand = new IioDeviceAttributeRead(swiot, "mode", nullptr);
		connect(readModeCommand, &scopy::Command::finished, this, &SwiotController::readModeCommandFinished,
			Qt::QueuedConnection);
		m_cmdQueue->enqueue(readModeCommand);
	} else {
		qDebug(CAT_SWIOT) << "Can't find swiot iio_device";
	}
}

void SwiotController::readModeCommandFinished(scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *mode = tcmd->getResult();
		bool runtime = (strcmp(mode, "runtime") == 0);
		setIsRuntimeCtx(runtime);
	} else {
		qDebug(CAT_SWIOT) << R"(Critical error: could not read mode attribute, error code:)"
				  << tcmd->getReturnCode();
	}
}

void SwiotController::setIsRuntimeCtx(bool runtimeCtx)
{
	m_isRuntimeCtx = runtimeCtx;
	Q_EMIT isRuntimeCtxChanged(m_isRuntimeCtx);
}
