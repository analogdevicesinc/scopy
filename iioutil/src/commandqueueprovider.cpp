#include "commandqueueprovider.h"

#include <QApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CMDQMGR, "CommandQueueProvider")

using namespace scopy;

CommandQueueRefCounter::CommandQueueRefCounter(struct iio_context *ctx)
{
	this->ctx = ctx;
	this->refcnt++;
	// TBD: automatically check the iio_context to see if multiple threads are possible (iiod vs tinyiiod)
	this->cmdQueue = new CommandQueue(1);
}

CommandQueueRefCounter::~CommandQueueRefCounter()
{
	if(this->cmdQueue) {
		delete cmdQueue;
		cmdQueue = nullptr;
	}
}

CommandQueueProvider *CommandQueueProvider::pinstance_{nullptr};
std::mutex CommandQueueProvider::mutex_;

CommandQueueProvider::CommandQueueProvider(QObject *parent) { qDebug(CAT_CMDQMGR) << "ctor"; }

CommandQueueProvider::~CommandQueueProvider() { qDebug(CAT_CMDQMGR) << "dtor"; }

CommandQueueProvider *scopy::CommandQueueProvider::GetInstance()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(pinstance_ == nullptr) {
		pinstance_ = new CommandQueueProvider(QApplication::instance());
	} else {
		qDebug(CAT_CMDQMGR) << "got instance from singleton";
	}
	return pinstance_;
}

CommandQueue *scopy::CommandQueueProvider::open(iio_context *ctx)
{
	std::lock_guard<std::mutex> lock(mutex_);
	CommandQueue *cmdQueue = nullptr;
	if(map.contains(ctx)) {
		map.value(ctx)->refcnt++;
		qDebug(CAT_CMDQMGR) << "opening command queue - found - refcnt++ = ";
	} else {
		CommandQueueRefCounter *cmdQRef = new CommandQueueRefCounter(ctx);
		if(cmdQRef->cmdQueue == nullptr) {
			qWarning(CAT_CMDQMGR) << " not a valid Command Queue";
			delete cmdQRef;
			return nullptr;
		}
		map.insert(ctx, cmdQRef);
		qDebug(CAT_CMDQMGR) << " created command queue - refcnt = " << map.value(ctx)->refcnt;
	}

	cmdQueue = map.value(ctx)->cmdQueue;
	return cmdQueue;
}

void CommandQueueProvider::close(struct iio_context *ctx)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if(map.contains(ctx)) {
		map.value(ctx)->refcnt--;
		qDebug(CAT_CMDQMGR) << " closing command queue - refcnt-- = " << map.value(ctx)->refcnt;
		if(map[ctx]->refcnt == 0) {
			delete map[ctx];
			map.remove(ctx);
			qDebug(CAT_CMDQMGR) << "removed from map";
		}
	} else {
		qInfo(CAT_CMDQMGR) << "not found in map. nop";
	}
}

#include "moc_commandqueueprovider.cpp"
