#include "iiocontextscanner.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

Q_LOGGING_CATEGORY(CAT_CTXSCANNER, "IIOContextScanner")

using namespace adiscope;
IIOContextScanner::IIOContextScanner(QObject *parent) : QObject(parent)
{
	qDebug(CAT_CTXSCANNER)<< "ctor ";
	t = new QTimer(this);
	scannerThread = new IIOContextScannerThread(this);
	connect(t,SIGNAL(timeout()),this,SLOT(startScanThread()));
}

IIOContextScanner::~IIOContextScanner() {
	stopScan();
	if(!scannerThread->isFinished()) {
		qDebug(CAT_CTXSCANNER)<< "Waiting to finish scanner thread - max 30 seconds";
		scannerThread->wait(THREAD_FINISH_TIMEOUT);
	}
	qDebug(CAT_CTXSCANNER)<< "dtor ";
}

void IIOContextScanner::startScan(int period, QString scanParams)
{
	if(!enabled) {
		scanFinishedSignal = connect(scannerThread,SIGNAL(scanFinished(QStringList)),this,SIGNAL(scanFinished(QStringList)));
		scannerThread->setScanParams(scanParams);
		scannerThread->start();
		t->start(period);
		enabled = true;
	}

}

void IIOContextScanner::startScanThread() {
	qDebug(CAT_CTXSCANNER)<<"Attempting to start scanner thread";
	if(scannerThread->isFinished()) {
		scannerThread->start();
	}
}


void IIOContextScanner::stopScan()
{
	if(enabled) {
		qDebug(CAT_CTXSCANNER)<<"Stopping scanner thread";
		disconnect(scanFinishedSignal);
		t->stop();
		enabled = false;
	}
}

 int IIOContextScanner::scan(QStringList *ctxs, QString scanParams) {
	qDebug(CAT_CTXSCANNER)<< "start scanning";
	struct iio_scan_context *scan_ctx = NULL;
	struct iio_context_info **info;
	int num_contexts;
	int ret;
	int i;

	QElapsedTimer et;
	et.start();
	if(scanParams.isEmpty()) {
		scan_ctx = iio_create_scan_context(NULL, 0);
	} else {
		scan_ctx = iio_create_scan_context(scanParams.toStdString().c_str(), 0);
	}

	if (!scan_ctx) {
		qWarning(CAT_CTXSCANNER)<< "no scan context - "  << errno << " " <<strerror(errno);
		return -1;
	}
	ret = iio_scan_context_get_info_list(scan_ctx, &info);
	if (ret < 0) {
		qWarning(CAT_CTXSCANNER)<< "iio_scan_context_get_info_list error - " << errno << " " <<strerror(errno);
		goto scan_err;
	}

	num_contexts = ret;

	qDebug(CAT_CTXSCANNER)<< "found " << num_contexts << "contexts in " << et.elapsed() << "miliseconds ";
	for (i = 0; i < num_contexts; i++) {
		ctxs->append(QString(iio_context_info_get_uri(info[i])));
	}
	iio_context_info_list_free(info);
	qDebug(CAT_CTXSCANNER)<<"scanned "<<*ctxs;

scan_err:
	iio_scan_context_destroy(scan_ctx);
	return ret;
}

/*
	ContextScanner *cs = new ContextScanner(this);
	ScannedContextCollector *scc = new ScannedContextCollector(this);
	connect(cs,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));
	cs->startScan(2000);
*/
