#ifndef IIOPINGTASK_H
#define IIOPINGTASK_H

#include <QThread>
#include <iio.h>
#include "scopyiioutil_export.h"
#include "task.h"

namespace adiscope {

class SCOPYIIOUTIL_EXPORT IIOPingTask : public Task {
	Q_OBJECT
public:
	IIOPingTask(iio_context *c, QObject *parent = nullptr);
	~IIOPingTask();
	virtual void run() override;

	static bool ping(iio_context *ctx);

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();
protected:
	iio_context *c;
	bool enabled;

};
}
#endif // IIOPINGTASK_H
