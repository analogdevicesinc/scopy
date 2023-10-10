#ifndef CYCLICALTASK_H
#define CYCLICALTASK_H

#include "scopy-iioutil_export.h"

#include <iio.h>

#include <QLoggingCategory>
#include <QObject>
#include <QThread>
#include <QTimer>

namespace scopy {
/**
 * @brief The CyclicalTask class
 * Implements a way to cyclically call QThreads
 */
class SCOPY_IIOUTIL_EXPORT CyclicalTask : public QObject
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
	QThread *task;
	const int THREAD_FINISH_TIMEOUT = 30000;
};
} // namespace scopy

#endif // CYCLICALTASK_H
