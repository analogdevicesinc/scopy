#ifndef CYCLICALTASK_H
#define CYCLICALTASK_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <iio.h>
#include "task.h"
#include "scopyiioutil_export.h"
#include <QLoggingCategory>

namespace adiscope {

class SCOPYIIOUTIL_EXPORT CyclicalTask : public QObject
{
	Q_OBJECT
public:
	CyclicalTask(Task *task, QObject *parent = nullptr);
	~CyclicalTask();
	void start(int period = 5000);
	void stop();

private Q_SLOTS:
	void startThread();
private:
	QTimer *t;
	bool enabled = false;
	Task* task;
	const int THREAD_FINISH_TIMEOUT = 30000;
};
}



#endif // CYCLICALTASK_H
