#include "adcacquisitionmanager.h"
#include <QSet>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ACQTREENODE,"AcqTreeNode")

using namespace scopy;
using namespace scopy::grutil;

GRIIOFloatChannelNode::GRIIOFloatChannelNode(GRIIOFloatChannelSrc *c, QObject *parent)
	: AcqTreeNode(c->getChannelName(), parent)
{
	m_src = c;
	m_signalPath = new GRSignalPath("time_" + c->getDeviceSrc()->deviceName() + c->getChannelName(), this);
	m_signalPath->append(c);
	m_scOff = new GRScaleOffsetProc(m_signalPath);
	m_signalPath->append(m_scOff);
	m_scOff->setOffset(0);
	m_scOff->setScale(1);
	m_signalPath->setEnabled(false);
}

GRIIOFloatChannelNode::~GRIIOFloatChannelNode() {}

GRSignalPath *scopy::GRIIOFloatChannelNode::signalPath() const { return m_signalPath; }

GRScaleOffsetProc *scopy::GRIIOFloatChannelNode::scOff() const { return m_scOff; }

GRIIOFloatChannelSrc *GRIIOFloatChannelNode::src() const
{
	return m_src;
}

GRIIODeviceSourceNode::GRIIODeviceSourceNode(GRIIODeviceSource *d, QObject *parent)
	: AcqTreeNode(d->deviceName(), parent)
{
	m_src = d;
}

GRIIODeviceSourceNode::~GRIIODeviceSourceNode() {}

GRIIODeviceSource *GRIIODeviceSourceNode::src()
{
	return m_src;
}

GRTopBlockNode::GRTopBlockNode(GRTopBlock *g, QObject *parent)
	: AcqTreeNode(g->name(), parent)
{
	m_data = g;
}

GRTopBlockNode::~GRTopBlockNode() {}

AcqTreeNode::AcqTreeNode(QString name, QObject *parent)
	: QObject(parent)
{
	m_name = name;
}

AcqTreeNode::~AcqTreeNode() {}

void AcqTreeNode::addTreeChild(AcqTreeNode *t) {
	m_treeChildren.append(t);
	connect(t, &AcqTreeNode::newChild, this, &AcqTreeNode::newChild);
	t->setTreeParent(this);
	for(AcqTreeNode *c : t->bfs()){
		Q_EMIT newChild(c);
	}

}

bool AcqTreeNode::removeTreeChild(AcqTreeNode *t) {
	bool b = m_treeChildren.removeAll(t);
	if(b) {
		// bfs delete all (?)
		Q_EMIT deletedChild(t);
		t->deleteLater(); // ???
	}
	return b;
}


void *AcqTreeNode::data() { return m_data; }

QList<AcqTreeNode *> AcqTreeNode::bfs()
{
	QList<AcqTreeNode*> list;
	QList<AcqTreeNode*> visited;
	visited.push_back(this);
	list.append(this);

	while (!list.empty()) {
		AcqTreeNode* current = list.front();
		list.pop_front();
		for (AcqTreeNode* child : current->m_treeChildren) {
			if (!visited.contains(child)) {
				visited.push_back(child);
				list.push_back(child);
			}
		}
	}
	return visited;
}

QString AcqTreeNode::name() const
{
	return m_name;
}

AcqTreeNode *AcqTreeNode::treeParent() const
{
	return m_treeParent;
}

void AcqTreeNode::setTreeParent(AcqTreeNode *newTreeParent)
{
	m_treeParent = newTreeParent;
}

QList<AcqTreeNode *> AcqTreeNode::treeChildren() const
{
	return m_treeChildren;
}
