#include "pingtask.h"

using namespace scopy;
PingTask::PingTask(QObject *parent)
	: QThread(parent)
{}

PingTask::~PingTask() {}

void PingTask::run() {}

#include "moc_pingtask.cpp"
