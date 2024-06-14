#ifndef IIOPINGTASK_H
#define IIOPINGTASK_H

#include "pingtask.h"
#include "scopy-iioutil_export.h"

#include <iio.h>

namespace scopy {
/**
 * @brief The IIOPingTask class
 * IIOPingTask verifies IIO connection and emits pingSuccess/pingFailed
 */
class SCOPY_IIOUTIL_EXPORT IIOPingTask : public PingTask
{
	Q_OBJECT
public:
	IIOPingTask(iio_context *c, QObject *parent = nullptr);
	~IIOPingTask();
	virtual void run() override;
	virtual bool ping() override;

protected:
	iio_context *m_ctx;
};
} // namespace scopy
#endif // IIOPINGTASK_H
