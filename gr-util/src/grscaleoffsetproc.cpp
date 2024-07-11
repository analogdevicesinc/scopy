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
		for(int i = 0;i<m_top->vlen();i++) {
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
	for(int i = 0;i<m_vlen;i++) {
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
