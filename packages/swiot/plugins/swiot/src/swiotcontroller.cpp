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

#include "swiot_logging_categories.h"

#include <component/device.h>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>

#include <qcorotask.h>

using namespace scopy;
using namespace scopy::swiot;

SwiotController::SwiotController(QString uri, QObject *parent)
	: QObject(parent)
	, m_swiotDevice(nullptr)
	, uri(uri)
	, m_isRuntimeCtx(false)
	, m_temperatureReadEn(false)
{
	m_temperatureTask = nullptr;
	temperatureTimer = nullptr;
	m_extPsTask = nullptr;
	powerSupplyTimer = nullptr;
	m_identifyTask = nullptr;
}

SwiotController::~SwiotController()
{
	if(m_identifyTask) {
		delete m_identifyTask;
		m_identifyTask = nullptr;
	}
	m_context = {};
}

void SwiotController::connectSwiot()
{
	m_context = component::Controller::context(uri);
	m_swiotDevice =
		m_context ? m_context->findChild<component::Device *>("swiot", Qt::FindDirectChildrenOnly) : nullptr;
}

void SwiotController::disconnectSwiot()
{
	m_swiotDevice = nullptr;
	m_context = {};
}

void SwiotController::startPowerSupplyTask(QString attribute)
{
	m_extPsTask = new ExternalPsReader(uri, attribute, this);
	powerSupplyTimer = new QTimer(this);
	powerSupplyTimer->setInterval(5000);
	connect(m_extPsTask, &ExternalPsReader::hasConnectedPowerSupply, this,
		&SwiotController::hasConnectedPowerSupply);
	connect(powerSupplyTimer, &QTimer::timeout, this, [this]() { m_extPsTask->readPowerSupply(); });
	m_extPsTask->readPowerSupply();
	powerSupplyTimer->start();
}

void SwiotController::stopPowerSupplyTask()
{
	if(powerSupplyTimer) {
		powerSupplyTimer->stop();
		powerSupplyTimer->deleteLater();
		powerSupplyTimer = nullptr;
	}
	if(m_extPsTask) {
		disconnect(m_extPsTask, &ExternalPsReader::hasConnectedPowerSupply, this,
			   &SwiotController::hasConnectedPowerSupply);
		m_extPsTask->deleteLater();
		m_extPsTask = nullptr;
	}
}

void SwiotController::startTemperatureTask()
{
	if(!m_isRuntimeCtx || m_temperatureReadEn) {
		return;
	}
	m_temperatureTask = new SwiotReadTemperatureTask(uri, this);
	temperatureTimer = new QTimer(this);
	temperatureTimer->setInterval(2000);
	connect(m_temperatureTask, &SwiotReadTemperatureTask::newTemperature, this, &SwiotController::readTemperature);
	connect(temperatureTimer, &QTimer::timeout, this, [this]() { m_temperatureTask->readTemperature(); });
	m_temperatureTask->readTemperature();
	temperatureTimer->start();
	m_temperatureReadEn = true;
}

void SwiotController::stopTemperatureTask()
{
	if(!m_isRuntimeCtx || !m_temperatureReadEn) {
		return;
	}
	if(temperatureTimer) {
		temperatureTimer->stop();
		temperatureTimer->deleteLater();
		temperatureTimer = nullptr;
	}
	if(m_temperatureTask) {
		disconnect(m_temperatureTask, &SwiotReadTemperatureTask::newTemperature, this,
			   &SwiotController::readTemperature);
		m_temperatureTask->deleteLater();
		m_temperatureTask = nullptr;
	}
	m_temperatureReadEn = false;
}

void SwiotController::identify()
{
	if(!m_identifyTask) {
		m_identifyTask = new SwiotIdentifyTask(uri, this);
	}
	m_identifyTask->identify();
}

QCoro::Task<void> SwiotController::writeModeAttribute(QString mode)
{
	if(!m_swiotDevice) {
		qDebug(CAT_SWIOT) << "Can't find swiot device";
		Q_EMIT writeModeFailed();
		co_return;
	}
	component::Attribute *attr =
		m_swiotDevice->findChild<component::Attribute *>("mode", Qt::FindDirectChildrenOnly);
	if(!attr || !attr->writeCapability()) {
		qDebug(CAT_SWIOT) << "Can't find swiot mode attribute";
		Q_EMIT writeModeFailed();
		co_return;
	}
	auto r = co_await attr->writeCapability()->writeAsync(mode);
	if(r) {
		qInfo(CAT_SWIOT) << R"(Successfully written swiot mode)";
		Q_EMIT modeAttributeChanged(mode);
	} else {
		qDebug(CAT_SWIOT) << R"(Error, could not change swiot mode)";
		Q_EMIT writeModeFailed();
	}
}

QCoro::Task<void> SwiotController::readModeAttribute()
{
	if(!m_swiotDevice) {
		qDebug(CAT_SWIOT) << "Can't find swiot device";
		co_return;
	}
	component::Attribute *attr =
		m_swiotDevice->findChild<component::Attribute *>("mode", Qt::FindDirectChildrenOnly);
	if(!attr || !attr->readCapability()) {
		co_return;
	}
	auto r = co_await attr->readCapability()->readAsync();
	if(r) {
		bool runtime = (attr->cachedValue() == "runtime");
		setIsRuntimeCtx(runtime);
	} else {
		qDebug(CAT_SWIOT) << R"(Critical error: could not read mode attribute)";
	}
}

void SwiotController::setIsRuntimeCtx(bool runtimeCtx)
{
	m_isRuntimeCtx = runtimeCtx;
	Q_EMIT isRuntimeCtxChanged(m_isRuntimeCtx);
}

#include "moc_swiotcontroller.cpp"
