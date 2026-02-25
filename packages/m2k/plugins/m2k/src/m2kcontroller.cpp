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

#include "m2kcontroller.h"

#include "m2kcommon.h"

#include <QFuture>
#include <QFutureWatcher>

#include <libm2k/m2kexceptions.hpp>

using namespace libm2k;
using namespace libm2k::analog;
using namespace libm2k::context;

using namespace scopy;
using namespace scopy::m2k;

M2kController::M2kController(QString uri, QObject *parent)
	: QObject(parent)
	, m_uri(uri)
	, m_identifyTask(nullptr)
	, m_m2k(nullptr)
	, m_tempTask(nullptr)
	, m_tempTimer(nullptr)

{
	m_calibFw = new QFutureWatcher<bool>(this);
	connect(m_calibFw, &QFutureWatcher<bool>::finished, this, &M2kController::onCalibFinished);
}

M2kController::~M2kController() {}

void M2kController::startTemperatureTask()
{
	m_tempTask = new M2kReadTemperatureTask(m_uri);
	m_tempTimer = new CyclicalTask(m_tempTask);
	connect(m_tempTask, SIGNAL(newTemperature(double)), this, SIGNAL(newTemperature(double)));
	m_tempTimer->start();
}

void M2kController::stopTemperatureTask()
{
	if(!m_tempTimer || !m_tempTask) {
		return;
	}
	m_tempTimer->stop();
	m_tempTask->requestInterruption();
	disconnect(m_tempTask, SIGNAL(newTemperature(double)), this, SIGNAL(newTemperature(double)));
}

void M2kController::connectM2k(libm2k::context::M2k *m2k)
{
	m_m2k = m2k;
	identify();
}

void M2kController::disconnectM2k()
{
	if(!m_m2k) {
		return;
	}
	if(m_calibFw && m_calibFw->isRunning()) {
		m_calibFw->waitForFinished();
	}
	if(m_identifyTask && m_identifyTask->isRunning()) {
		m_identifyTask->requestInterruption();
	}
	m_m2k = nullptr;
}

void M2kController::identify()
{
	if(!m_identifyTask) {
		m_identifyTask = new M2kIdentifyTask(m_uri);
		m_identifyTask->start();
		connect(m_identifyTask, &QThread::finished, this, [=]() {
			delete m_identifyTask;
			m_identifyTask = nullptr;
		});
	}
}

void M2kController::initialCalibration()
{
	if(!m_m2k->isCalibrated()) {
		calibrate();
	} else {
		// already calibrated / or skipped
	}
}

void M2kController::calibrate()
{
	if(m_calibFw->isRunning()) {
		qWarning(CAT_M2KPLUGIN) << "Calibration already in progress!";
		return;
	}
	QFuture<bool> f = QtConcurrent::run(std::bind(&libm2k::context::M2k::calibrate, m_m2k));
	m_calibFw->setFuture(f);
	Q_EMIT calibrationStarted();
}

void M2kController::onCalibFinished()
{
	if(!m_m2k) {
		return;
	}
	try {
		if(m_calibFw->result()) {
			Q_EMIT calibrationSuccess();
		} else {
			Q_EMIT calibrationFailed();
		}
	} catch(...) {
		Q_EMIT calibrationFailed();
		qWarning(CAT_M2KPLUGIN) << "An exception occurred during calibration!";
	}
	Q_EMIT calibrationFinished();
}
