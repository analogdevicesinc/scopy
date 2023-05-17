#include "grproxyblock.h"
#include "grtopblock.h"
#include "grlog.h"

using namespace scopy::grutil;
GRProxyBlock::GRProxyBlock(QObject *parent) : QObject(parent), m_enabled(true) {}

GRProxyBlock::~GRProxyBlock() {}

void GRProxyBlock::build_blks(GRTopBlock *top)
{

}

void GRProxyBlock::destroy_blks(GRTopBlock *top)
{
	start_blk.clear();
}

void GRProxyBlock::connect_blk(GRTopBlock *top, GRProxyBlock *src) {
	if(src == nullptr) // block is a source
		return;
	int nrOfOutputs = src->getGrEndPoint()->output_signature()->max_streams();
	for(int i = 0; i < nrOfOutputs;i++) {

		int j = 0;
		int input = i;

		for(j=0;j<start_blk.count();j++) {
			int nrinputs = start_blk[j]->input_signature()->max_streams();
			if(input < nrinputs)
				break;
			input -= nrinputs;
		}
		top->connect(src->getGrEndPoint(),i,start_blk[j],input);
	}
}

void GRProxyBlock::disconnect_blk(GRTopBlock *top) {
	for(int i=0;i<start_blk.count();i++)
		top->getGrBlock()->disconnect(start_blk[i]);
}

void GRProxyBlock::setEnabled(bool v) {
	if(m_enabled == v)
		return;

	m_enabled = v;
	blockSignals(false); // make sure request rebuild is sent
	Q_EMIT requestRebuild();
	blockSignals(!v); // to prevent rebuilding from non-enabled blocks - maybe
}

bool GRProxyBlock::enabled() { return m_enabled; }

bool GRProxyBlock::built()
{
	return (!start_blk.isEmpty()) || (end_blk != nullptr);
}

QList<gr::basic_block_sptr> GRProxyBlock::getGrStartPoint() { return start_blk; }
gr::basic_block_sptr GRProxyBlock::getGrEndPoint() { return end_blk; }



