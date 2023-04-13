#ifndef IIOPINGTASK_H
#define IIOPINGTASK_H

#include <QThread>
#include <iio.h>
#include "scopyiioutil_export.h"

namespace scopy {
/**
 * @brief The IIOPingTask class
 * IIOPingTask verifies IIO connection and emits pingSuccess/pingFailed
 */
class SCOPYIIOUTIL_EXPORT IIOPingTask : public QThread {
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
