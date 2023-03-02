#include "cyclicaltask.h"
#include "task.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

Q_LOGGING_CATEGORY(CAT_CYCLICALTASK, "CyclicalTask")

using namespace adiscope;
CyclicalTask::CyclicalTask(Task *task, QObject *parent) : QObject(parent)
{
	qDebug(CAT_CYCLICALTASK)<< "ctor ";
	t = new QTimer(this);
	this->task = task;
	this->task->setParent(this);
	connect(t,SIGNAL(timeout()),this,SLOT(startThread()));
}

CyclicalTask::~CyclicalTask() {
	stop();
	if(!task->isFinished()) {
		qDebug(CAT_CYCLICALTASK)<< "Waiting to finish thread - max 30 seconds";
		task->wait(THREAD_FINISH_TIMEOUT);
	}
	qDebug(CAT_CYCLICALTASK)<< "dtor ";
}

void CyclicalTask::start(int period)
{
	if(!enabled) {
		task->start();
		t->start(period);
		enabled = true;
	}

}

void CyclicalTask::startThread() {
	qDebug(CAT_CYCLICALTASK)<<"Attempting to start thread";
	if(task->isFinished()) {
		task->start();
	}
}


void CyclicalTask::stop()
{
	if(enabled) {
		qDebug(CAT_CYCLICALTASK)<<"Stopping scanner thread";
		t->stop();		
		enabled = false;
	}
}

/*
	ContextScanner *cs = new ContextScanner(this);
	ScannedContextCollector *scc = new ScannedContextCollector(this);
	connect(cs,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));
	cs->startScan(2000);
*/
