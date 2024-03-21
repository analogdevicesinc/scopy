#include "adcacquisitionmanager.h"

using namespace scopy;
using namespace scopy::grutil;

GRIIOFloatChannelNode::GRIIOFloatChannelNode(GRIIOChannel *c, QObject *parent) : AcqTreeNode(c->getChannelName(), parent) {
	m_data = c;
	m_signalPath = new GRSignalPath("time_" + c->getDeviceSrc()->deviceName() + c->getChannelName(), this);
	m_signalPath->append(c);
	m_scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(m_scOff);
	m_scOff->setOffset(0);
	m_scOff->setScale(1);
	m_signalPath->setEnabled(false);
}

GRIIOFloatChannelNode::~GRIIOFloatChannelNode(){}

GRSignalPath *scopy::GRIIOFloatChannelNode::signalPath() const {
	return m_signalPath;
}

GRScaleOffsetProc *scopy::GRIIOFloatChannelNode::scOff() const {
	return m_scOff;
}

GRIIODeviceSourceNode::GRIIODeviceSourceNode(GRIIODeviceSource *d, QObject *parent) : AcqTreeNode(d->deviceName(), parent) {
	m_data = d;
}

GRIIODeviceSourceNode::~GRIIODeviceSourceNode(){}

GRTopBlockNode::GRTopBlockNode(GRTopBlock *g, QObject *parent) : AcqTreeNode(g->name(),parent) {
	m_data = g;
}

GRTopBlockNode::~GRTopBlockNode() {}

AcqTreeNode::AcqTreeNode(QString name, QObject *parent) : QObject(parent) { m_name = name; }

AcqTreeNode::~AcqTreeNode() { }

void AcqTreeNode::addChild(AcqTreeNode *t) { m_children.append(t); }

bool AcqTreeNode::removeChild(AcqTreeNode *t) { return m_children.removeAll(t); }

AcqTree *AcqTreeNode::tree() { return m_tree;}

void *AcqTreeNode::data() {
	return m_data;
}

AcqTree::AcqTree(QObject *parent) : QObject(parent) {}

AcqTree::~AcqTree() {

}
