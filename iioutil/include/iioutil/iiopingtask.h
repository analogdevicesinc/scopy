#ifndef IIOPINGTASK_H
#define IIOPINGTASK_H

#include "scopy-iioutil_export.h"

#include <iio.h>

#include <QThread>

namespace scopy {
/**
 * @brief The IIOPingTask class
 * IIOPingTask verifies IIO connection and emits pingSuccess/pingFailed
 */
class SCOPY_IIOUTIL_EXPORT IIOPingTask : public QThread
{
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
} // namespace scopy
#endif // IIOPINGTASK_H
