#include "iiocontextscannerthread.h"
#include "iiocontextscanner.h"

using namespace adiscope;

IIOContextScannerThread::IIOContextScannerThread(QObject *parent) : QThread(parent) {}
IIOContextScannerThread::~IIOContextScannerThread() {};

void IIOContextScannerThread::run()
{
	QStringList ctxs;
	int ret = IIOContextScanner::scan(&ctxs,scanParams);
	if( ret >= 0 )
		Q_EMIT scanFinished(ctxs);
}

void IIOContextScannerThread::setScanParams(QString s) {
	scanParams = s;
}
