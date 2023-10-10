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
	if(add)
		add->set_k(m_offset);
}

void GRScaleOffsetProc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL) << "Building GRScaleOffsetProc";
	mul = gr::blocks::multiply_const_ff::make(m_scale);
	add = gr::blocks::add_const_ff::make(m_offset);
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
