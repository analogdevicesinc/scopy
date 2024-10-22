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

#include "grscaleoffsetproc.h"

#include "grlog.h"
#include "grtopblock.h"

using namespace scopy::grutil;
GRScaleOffsetProc::GRScaleOffsetProc(QObject *parent)
	: GRProxyBlock(parent)
{}

void GRScaleOffsetProc::setScale(double sc)
{
	m_scale = sc;
	if(mul)
		mul->set_k(m_scale);
}

void GRScaleOffsetProc::setOffset(double off)
{
	m_offset = off;
	if(add) {
		std::vector<float> k;
		for(int i = 0; i < m_top->vlen(); i++) {
			k.push_back(m_offset);
		}
		add->set_k(k);
	}
}

void GRScaleOffsetProc::build_blks(GRTopBlock *top)
{
	size_t m_vlen = top->vlen();
	m_top = top;

	qDebug(SCOPY_GR_UTIL) << "Building GRScaleOffsetProc";
	mul = gr::blocks::multiply_const_ff::make(m_scale, m_vlen);

	std::vector<float> k;
	for(int i = 0; i < m_vlen; i++) {
		k.push_back(m_offset);
	}

	add = gr::blocks::add_const_v<float>::make(k);
	top->connect(mul, 0, add, 0);
	start_blk.append(mul);
	end_blk = add;
}

void GRScaleOffsetProc::destroy_blks(GRTopBlock *top)
{
	end_blk = nullptr;
	mul = nullptr;
	add = nullptr;
	start_blk.clear();
}
