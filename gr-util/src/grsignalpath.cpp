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

#include "grsignalpath.h"

#include "grlog.h"
#include "grtopblock.h"

using namespace scopy::grutil;

GRSignalPath::GRSignalPath(QString name, QObject *parent)
	: GRProxyBlock(parent)
	, m_name(name)
{}

void GRSignalPath::append(GRProxyBlock *p)
{
	list.append(p);
	connect(p, SIGNAL(requestRebuild()), this, SIGNAL(requestRebuild()));
}

void GRSignalPath::build_blks() {}

void GRSignalPath::destroy_blks() {}

QList<gr::basic_block_sptr> GRSignalPath::getGrStartPoint()
{
	for(int i = 0; i < list.count(); i++) {
		auto blk = list[i];
		if(blk->enabled()) {
			return blk->getGrStartPoint();
		}
	}
	return QList<gr::basic_block_sptr>();
}

gr::basic_block_sptr GRSignalPath::getGrEndPoint()
{
	for(int i = list.count() - 1; i >= 0; i--) {
		auto blk = list[i];
		if(blk->enabled()) {
			return blk->getGrEndPoint();
		}
	}
	return nullptr;
}

void GRSignalPath::connect_blk(GRTopBlock *top, GRProxyBlock *src)
{
	qDebug(SCOPY_GR_UTIL) << "Start connecting GRSignalPath" << name();
	GRProxyBlock *prevBlk = src;
	for(GRProxyBlock *blk : qAsConst(list)) {
		if(blk->enabled() && !blk->built() ||
		   (blk == list[0] && list.count() > 1)) // - an enabled signal path always needs a source enabled -
							 // unless it's the only one I guess */ )
		{
			blk->build_blks(top);
			blk->connect_blk(top, prevBlk);
			prevBlk = blk;
		}
	}
	qDebug(SCOPY_GR_UTIL) << "End connecting GRSignalPath";
}

void GRSignalPath::disconnect_blk(GRTopBlock *top)
{
	for(GRProxyBlock *blk : qAsConst(list)) {
		if(blk->built()) {
			blk->disconnect_blk(top);
			blk->destroy_blks(top);
		}
	}
}

QString GRSignalPath::name() const { return m_name; }

QList<GRProxyBlock *> GRSignalPath::path() { return list; }

#include "moc_grsignalpath.cpp"
