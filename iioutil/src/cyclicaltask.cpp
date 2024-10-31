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

#include "cyclicaltask.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CYCLICALTASK, "CyclicalTask")

using namespace scopy;
CyclicalTask::CyclicalTask(QThread *task, QObject *parent)
	: QObject(parent)
{
	qDebug(CAT_CYCLICALTASK) << "ctor ";
	t = new QTimer(this);
	this->task = task;
	this->task->setParent(this);
	connect(t, SIGNAL(timeout()), this, SLOT(startThread()));
}

CyclicalTask::~CyclicalTask()
{
	stop();
	if(!task->isFinished()) {
		qDebug(CAT_CYCLICALTASK) << "Waiting to finish thread - max 30 seconds";
		task->wait(THREAD_FINISH_TIMEOUT);
	}
	qDebug(CAT_CYCLICALTASK) << "dtor ";
}

void CyclicalTask::start(int period)
{
	if(!enabled) {
		task->start();
		t->start(period);
		enabled = true;
	}
}

void CyclicalTask::startThread()
{
	qDebug(CAT_CYCLICALTASK) << "Attempting to start thread";
	if(task->isFinished()) {
		task->start();
	}
}

void CyclicalTask::stop()
{
	if(enabled) {
		qDebug(CAT_CYCLICALTASK) << "Stopping scanner thread";
		t->stop();
		if(!task->isFinished()) {
			task->wait(THREAD_FINISH_TIMEOUT);
		}
		enabled = false;
	}
}

/*
	ContextScanner *cs = new ContextScanner(this);
	ScannedContextCollector *scc = new ScannedContextCollector(this);
	connect(cs,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));
	cs->startScan(2000);
*/

#include "moc_cyclicaltask.cpp"
