#include "contextscanner.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QElapsedTimer>

Q_LOGGING_CATEGORY(CAT_CTXSCANNER, "ContextScanner")

using namespace adiscope;
ContextScanner::ContextScanner(QObject *parent) : QObject(parent)
{
	qDebug(CAT_CTXSCANNER)<< "ctor ";
	t = new QTimer(this);
	connect(t, &QTimer::timeout,this,&ContextScanner::scan);
}

ContextScanner::~ContextScanner() {
	qDebug(CAT_CTXSCANNER)<< "dtor ";
}

void ContextScanner::startScan(int period, bool now)
{
	if(now)
	{
		scan();
	}
	// move to thread
	t->start(period);
}

void ContextScanner::stopScan()
{
	t->stop();
}

void ContextScanner::setScanParams(QString s)
{
	// mutex (?)
	scanParams = s;
}

void ContextScanner::scan()
{
	qDebug(CAT_CTXSCANNER)<< "start scanning";
	struct iio_scan_context *scan_ctx = NULL;
	struct iio_context_info **info;
	int num_contexts;
	int ret;
	int i;
	QStringList ctxs;

	QElapsedTimer et;
	et.start();
	if(scanParams.isEmpty()) {
		scan_ctx = iio_create_scan_context(NULL, 0);
	} else {
		scan_ctx = iio_create_scan_context(scanParams.toStdString().c_str(), 0);
	}

	if (!scan_ctx) {
		qWarning(CAT_CTXSCANNER)<< "no scan context - "  << errno << " " <<strerror(errno);
		return ;
	}
	ret = iio_scan_context_get_info_list(scan_ctx, &info);
	if (ret < 0) {
		qWarning(CAT_CTXSCANNER)<< "iio_scan_context_get_info_list error - " << errno << " " <<strerror(errno);
		goto scan_err;
		return ;
	}

	num_contexts = ret;

	qDebug(CAT_CTXSCANNER)<< "found " << num_contexts << "contexts in " << et.elapsed() << "miliseconds ";
	for (i = 0; i < num_contexts; i++) {
		ctxs.append(QString(iio_context_info_get_uri(info[i])));
	}
	iio_context_info_list_free(info);
	qDebug(CAT_CTXSCANNER)<<ctxs;
	// get results safely ?
	Q_EMIT scanFinished(ctxs);

scan_err:
	iio_scan_context_destroy(scan_ctx);
	return ;

}

/*
	auto cm = ContextManager::GetInstance();
	ContextScanner *cs = new ContextScanner(this);
	ScannedContextCollector *scc = new ScannedContextCollector(this);
	connect(cs,SIGNAL(scanFinished(QStringList)),scc,SLOT(update(QStringList)));
	cs->startScan(2000);
*/
