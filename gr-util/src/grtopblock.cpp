#include "grtopblock.h"

#include "grlog.h"

#include <QtConcurrent>

Q_LOGGING_CATEGORY(SCOPY_GR_UTIL, "GRManager")

using namespace scopy::grutil;
GRTopBlock::GRTopBlock(QString name, QObject *parent)
	: QObject(parent)
	, running(false)
	, built(false)
{
	static int topblockid = 0;
	QString topblockname = m_name + QString::number(topblockid);
	topblockid++;
	qInfo() << "building" << topblockname;
	top = gr::make_top_block(topblockname.toStdString());
}

GRTopBlock::~GRTopBlock() {}

void GRTopBlock::registerSignalPath(GRSignalPath *sig)
{
	m_signalPaths.append(sig);
	QObject::connect(sig, SIGNAL(requestRebuild()), this, SLOT(rebuild()));
	rebuild();
}

void GRTopBlock::unregisterSignalPath(GRSignalPath *sig)
{
	m_signalPaths.removeAll(sig);
	QObject::disconnect(sig, SIGNAL(requestRebuild()), this, SLOT(rebuild()));
	rebuild();
}

void GRTopBlock::registerIIODeviceSource(GRIIODeviceSource *dev)
{
	if(m_iioDeviceSources.contains(dev))
		return;
	m_iioDeviceSources.append(dev);
	rebuild();
}

void GRTopBlock::unregisterIIODeviceSource(GRIIODeviceSource *dev)
{
	m_iioDeviceSources.removeAll(dev);
	rebuild();
}

void GRTopBlock::build()
{

	top->disconnect_all();
	Q_EMIT aboutToBuild();

	for(GRSignalPath *sig : qAsConst(m_signalPaths)) {
		if(sig->enabled()) {
			sig->connect_blk(this, nullptr);
		}
	}
	for(GRIIODeviceSource *dev : qAsConst(m_iioDeviceSources)) {
		dev->build_blks(this);
		dev->connect_blk(this, nullptr);
	}
	Q_EMIT builtSignalPaths();
	built = true;
}

void GRTopBlock::teardown()
{
	Q_EMIT aboutToTeardown();
	built = false;

	for(GRIIODeviceSource *dev : qAsConst(m_iioDeviceSources)) {
		if(dev->built()) {
			dev->disconnect_blk(this);
			dev->destroy_blks(this);
		}
	}

	for(GRSignalPath *sig : qAsConst(m_signalPaths)) {
		sig->disconnect_blk(this);
	}

	top->disconnect_all();
	Q_EMIT teardownSignalPaths();
}

void GRTopBlock::start()
{
	qInfo(SCOPY_GR_UTIL) << "Starting top block";
	running = true;
	Q_EMIT aboutToStart();
	top->start();
	Q_EMIT started();

	//	QtConcurrent::run([=]() { - this causes a race condition
	//		top->wait();
	//		Q_EMIT finished();
	//	});
}

void GRTopBlock::stop()
{
	qInfo(SCOPY_GR_UTIL) << "Stopping top block";
	Q_EMIT aboutToStop();
	running = false;
	top->stop();
	top->wait(); // ??
	Q_EMIT stopped();
}

void GRTopBlock::run()
{
	start();
	top->wait();
}

void GRTopBlock::rebuild()
{
	qInfo(SCOPY_GR_UTIL) << "Rebuilding top block";
	bool wasRunning = false;
	if(running) {
		wasRunning = true;
		stop();
	}

	if(built) {
		teardown();
		build();
	}

	if(wasRunning) {
		start();
	}
}

void GRTopBlock::connect(gr::basic_block_sptr src, int srcPort, gr::basic_block_sptr dst, int dstPort)
{
	qDebug(SCOPY_GR_UTIL) << "Connecting " << QString::fromStdString(src->symbol_name()) << ":" << srcPort << "to"
			      << QString::fromStdString(dst->symbol_name()) << ":" << dstPort;
	top->connect(src, srcPort, dst, dstPort);
}

gr::top_block_sptr GRTopBlock::getGrBlock() { return top; }
