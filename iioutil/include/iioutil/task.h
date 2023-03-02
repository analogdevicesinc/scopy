#ifndef TASK_H
#define TASK_H

#include <QThread>
#include "scopyiioutil_export.h"

class SCOPYIIOUTIL_EXPORT Task : public QThread {

	Q_OBJECT
public:
	Task(QObject *parent = nullptr) : QThread(parent) {}
	virtual ~Task() {};
	virtual void run() override = 0;
	virtual void start(QThread::Priority = QThread::InheritPriority) = 0;
	virtual void stop() = 0;

};


#endif // TASK_H
