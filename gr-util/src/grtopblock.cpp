/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "grtopblock.h"

#include "grlog.h"

#include <QtConcurrent>

Q_LOGGING_CATEGORY(SCOPY_GR_UTIL, "GRManager")

using namespace scopy::grutil;
GRTopBlock::GRTopBlock(QString name, QObject *parent)
	: QObject(parent)
	, running(false)
	, built(false)
	, m_suspended(false)
{
	m_name = name;
	static int topblockid = 0;
	QString topblockname = m_name + QString::number(topblockid);
	topblockid++;
	qInfo() << "building" << topblockname;
	top = gr::make_top_block(topblockname.toStdString(), true);
	QObject::connect(this, SIGNAL(requestRebuild()), this, SLOT(rebuild()), Qt::QueuedConnection);
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

void GRTopBlock::setVLen(size_t vlen) { m_vlen = vlen; }

size_t GRTopBlock::vlen() { return m_vlen; }

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

QString GRTopBlock::name() const { return m_name; }

void GRTopBlock::suspendBuild() { m_suspended = true; }

void GRTopBlock::unsuspendBuild()
{
	m_suspended = false;
	rebuild();
}

void GRTopBlock::rebuild()
{
	if(m_suspended)
		return;
	qInfo(SCOPY_GR_UTIL) << QObject::sender();
	qInfo(SCOPY_GR_UTIL) << "Request rebuild";
	bool wasRunning = false;
	if(running) {
		qInfo(SCOPY_GR_UTIL) << "Stopping";
		wasRunning = true;
		stop();
	}

	if(built) {
		qInfo(SCOPY_GR_UTIL) << "building";
		teardown();
		build();
	}

	if(wasRunning) {
		qInfo(SCOPY_GR_UTIL) << "starting";
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

#include "moc_grtopblock.cpp"
