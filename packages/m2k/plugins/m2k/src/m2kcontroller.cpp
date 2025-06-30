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
	, uri(uri)
{
	identifyTask = nullptr;
	m_m2k = nullptr;
}

M2kController::~M2kController() {}

void M2kController::startTemperatureTask()
{
	tempTask = new M2kReadTemperatureTask(uri);
	tempTimer = new CyclicalTask(tempTask);
	connect(tempTask, SIGNAL(newTemperature(double)), this, SIGNAL(newTemperature(double)));
	tempTimer->start();
}

void M2kController::stopTemperatureTask()
{
	tempTimer->stop();
	tempTask->requestInterruption();
	disconnect(tempTask, SIGNAL(newTemperature(double)), this, SIGNAL(newTemperature(double)));
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
	try {
		if(identifyTask && identifyTask->isRunning()) {
			identifyTask->requestInterruption();
		}
	} catch(std::exception &ex) {
		qDebug(CAT_M2KPLUGIN) << ex.what();
	}
	m_m2k = nullptr;
}

void M2kController::identify()
{
	if(!identifyTask) {
		identifyTask = new M2kIdentifyTask(uri);
		identifyTask->start();
		connect(identifyTask, &QThread::finished, this, [=]() {
			delete identifyTask;
			identifyTask = nullptr;
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
	QFutureWatcher<bool> *fw = new QFutureWatcher<bool>(this);
	QFuture<bool> f = QtConcurrent::run(std::bind(&libm2k::context::M2k::calibrate, m_m2k));
	connect(fw, &QFutureWatcher<bool>::finished, this, [=]() {
		try {
			if(fw->result()) {
				Q_EMIT calibrationSuccess();
			} else {
				Q_EMIT calibrationFailed();
			}
		} catch(...) {
			Q_EMIT calibrationFailed();
			qWarning(CAT_M2KPLUGIN) << "An exception occurred during CALIBRATION!";
		}
		Q_EMIT calibrationFinished();
		fw->deleteLater();
	});
	fw->setFuture(f);

	Q_EMIT calibrationStarted();
}
