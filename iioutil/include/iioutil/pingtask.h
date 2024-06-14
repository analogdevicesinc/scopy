#ifndef PINGTASK_H
#define PINGTASK_H

#include "scopy-iioutil_export.h"
#include <QThread>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT PingTask : public QThread
{
	Q_OBJECT
public:
	PingTask(QObject *parent);
	~PingTask();

	virtual void run() override;
	virtual bool ping() = 0;

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
};
} // namespace scopy

#endif // PINGTASK_H
