#ifndef COMMANDQUEUEPROVIDER_H
#define COMMANDQUEUEPROVIDER_H

#include "scopy-iioutil_export.h"
#include "commandqueue.h"
#include <QObject>
#include <iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandQueueRefCounter {
public:
	CommandQueueRefCounter(struct iio_context *ctx);
	~CommandQueueRefCounter();
	struct iio_context *ctx;
	CommandQueue *cmdQueue = nullptr;
	int refcnt = 0;
};

class SCOPY_IIOUTIL_EXPORT CommandQueueProvider : public QObject
{
Q_OBJECT
protected:
	CommandQueueProvider(QObject *parent = nullptr);
	~CommandQueueProvider();
	QString name;
public:

	CommandQueueProvider(CommandQueueProvider &other) = delete;
	void operator=(const CommandQueueProvider &) = delete;

	static CommandQueueProvider *GetInstance();
	CommandQueue* open(struct iio_context *ctx);
	void close(struct iio_context *ctx);

private:
	static CommandQueueProvider * pinstance_;
	static std::mutex mutex_;
	QMap<struct iio_context*, CommandQueueRefCounter*> map;

};
}

#endif
