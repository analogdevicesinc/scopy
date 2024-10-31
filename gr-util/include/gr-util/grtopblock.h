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

#ifndef GRTOPBLOCK_H
#define GRTOPBLOCK_H

#include "griiodevicesource.h"
#include "grsignalpath.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/top_block.h>

#include <QObject>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRTopBlock : public QObject
{
	Q_OBJECT
public:
	GRTopBlock(QString name, QObject *parent = nullptr);
	~GRTopBlock();
	void registerSignalPath(GRSignalPath *path);
	void unregisterSignalPath(GRSignalPath *path);
	QList<GRSignalPath *> signalPaths() { return m_signalPaths; }

	void registerIIODeviceSource(GRIIODeviceSource *);
	void unregisterIIODeviceSource(GRIIODeviceSource *);

	void setVLen(size_t vlen);
	size_t vlen();

	void connect(gr::basic_block_sptr src, int srcPort, gr::basic_block_sptr dst, int dstPort);

	gr::top_block_sptr getGrBlock();

	QString name() const;

Q_SIGNALS:
	void aboutToBuild();
	void builtSignalPaths();
	void aboutToTeardown();
	void teardownSignalPaths();
	void aboutToStart();
	void started();
	void aboutToStop();
	void stopped();
	void finished();
	void requestRebuild();

public Q_SLOTS:
	void build();
	void teardown();
	void rebuild();
	void start();
	void stop();
	void run();
	void suspendBuild();
	void unsuspendBuild();

private:
	bool m_suspended;
	gr::top_block_sptr top;
	QString m_name;
	bool running;
	bool built;
	size_t m_vlen;
	QList<GRSignalPath *> m_signalPaths;
	QList<GRIIODeviceSource *> m_iioDeviceSources;
};

} // namespace scopy::grutil
#endif // GRTOPBLOCK_H
