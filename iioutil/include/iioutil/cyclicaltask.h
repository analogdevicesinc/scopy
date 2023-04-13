#ifndef CYCLICALTASK_H
#define CYCLICALTASK_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <iio.h>
#include "scopyiioutil_export.h"
#include <QLoggingCategory>

namespace scopy {
/**
 * @brief The CyclicalTask class
 * Implements a way to cyclically call QThreads
 */
class SCOPYIIOUTIL_EXPORT CyclicalTask : public QObject
{
	Q_OBJECT
public:
	CyclicalTask(QThread *task, QObject *parent = nullptr);
	~CyclicalTask();
	void start(int period = 5000);
	void stop();

private Q_SLOTS:
	void startThread();
private:
	QTimer *t;
	bool enabled = false;
	QThread* task;
	const int THREAD_FINISH_TIMEOUT = 30000;
};
}



#endif // CYCLICALTASK_H
