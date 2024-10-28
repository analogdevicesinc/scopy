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

#ifndef GRPROXYBLOCK_H
#define GRPROXYBLOCK_H

#include "scopy-gr-util_export.h"

#include <gnuradio/basic_block.h>

#include <QObject>

namespace scopy::grutil {

class GRTopBlock;

class SCOPY_GR_UTIL_EXPORT GRProxyBlock : public QObject
{
	Q_OBJECT
public:
	GRProxyBlock(QObject *parent = nullptr);
	virtual ~GRProxyBlock();

	virtual void build_blks(GRTopBlock *top);
	virtual void destroy_blks(GRTopBlock *top);
	virtual void connect_blk(GRTopBlock *top, GRProxyBlock *src);
	virtual void disconnect_blk(GRTopBlock *top);

	void setEnabled(bool v);
	bool enabled();
	bool built();
	virtual QList<gr::basic_block_sptr> getGrStartPoint();
	virtual gr::basic_block_sptr getGrEndPoint();

Q_SIGNALS:
	void requestRebuild();

protected:
	QList<gr::basic_block_sptr> start_blk; // QList of (?)
	gr::basic_block_sptr end_blk;
	bool m_enabled;
	//	bool m_built;
};

} // namespace scopy::grutil

#endif // GRPROXYBLOCK_H
