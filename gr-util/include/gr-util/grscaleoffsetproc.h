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

#ifndef GRSCALEOFFSETPROC_H
#define GRSCALEOFFSETPROC_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/add_const_v.h>
#include <gnuradio/blocks/multiply_const.h>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRScaleOffsetProc : public GRProxyBlock
{
public:
	GRScaleOffsetProc(QObject *parent = nullptr);
	void setScale(double sc);
	void setOffset(double off);
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	gr::blocks::add_const_v<float>::sptr add;
	gr::blocks::multiply_const_ff::sptr mul;
	double m_scale;
	double m_offset;
	GRTopBlock *m_top;
};
} // namespace scopy::grutil
#endif // GRSCALEOFFSETPROC_H
