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

#ifndef GRSIGNALPATH_H
#define GRSIGNALPATH_H
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <QObject>

namespace scopy::grutil {
class GRTopBlock;
class GRProxyBlock;

class SCOPY_GR_UTIL_EXPORT GRSignalPath : public GRProxyBlock
{
	Q_OBJECT
public:
	GRSignalPath(QString name, QObject *parent = nullptr);
	void append(GRProxyBlock *p);
	void build_blks();
	void destroy_blks();
	virtual QList<gr::basic_block_sptr> getGrStartPoint();
	virtual gr::basic_block_sptr getGrEndPoint();
	virtual void connect_blk(GRTopBlock *top, GRProxyBlock *src);
	virtual void disconnect_blk(GRTopBlock *top);
	QString name() const;
	QList<GRProxyBlock *> path();

protected:
	QList<GRProxyBlock *> list;
	QString m_name;
};
} // namespace scopy::grutil
#endif // GRSIGNALPATH_H
