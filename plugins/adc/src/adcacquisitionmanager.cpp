#include "adcacquisitionmanager.h"
#include <QSet>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ACQTREENODE, "AcqTreeNode")

using namespace scopy;
using namespace scopy::grutil;

GRIIOFloatChannelNode::GRIIOFloatChannelNode(GRTopBlockNode *top, GRIIOFloatChannelSrc *c, QObject *parent)
	: AcqTreeNode(c->getChannelName(), parent)
	, m_top(top)
{
	m_src = c;
}

GRIIOFloatChannelNode::~GRIIOFloatChannelNode() {}

GRIIOFloatChannelSrc *GRIIOFloatChannelNode::src() const { return m_src; }

GRTopBlockNode *GRIIOFloatChannelNode::top() const { return m_top; }

GRIIODeviceSourceNode::GRIIODeviceSourceNode(GRTopBlockNode *top, GRIIODeviceSource *d, QObject *parent)
	: AcqTreeNode(d->deviceName(), parent)
	, m_top(top)
{
	m_src = d;
}

GRIIODeviceSourceNode::~GRIIODeviceSourceNode() {}

GRIIODeviceSource *GRIIODeviceSourceNode::src() const { return m_src; }

GRTopBlockNode *GRIIODeviceSourceNode::top() const { return m_top; }

GRTopBlockNode::GRTopBlockNode(GRTopBlock *g, QObject *parent)
	: AcqTreeNode(g->name(), parent)
	, m_src(g)
{
	m_data = g;
}

GRTopBlockNode::~GRTopBlockNode() {}

GRTopBlock *GRTopBlockNode::src() const { return m_src; }

AcqTreeNode::AcqTreeNode(QString name, QObject *parent)
	: QObject(parent)
{
	m_name = name;
}

AcqTreeNode::~AcqTreeNode() {}

void AcqTreeNode::addTreeChild(AcqTreeNode *t)
{
	m_treeChildren.append(t);
	connect(t, &AcqTreeNode::newChild, this, &AcqTreeNode::newChild);
	t->setTreeParent(this);
	for(AcqTreeNode *c : t->bfs()) {
		Q_EMIT newChild(c);
	}
}

bool AcqTreeNode::removeTreeChild(AcqTreeNode *t)
{
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
	QList<AcqTreeNode *> list;
	QList<AcqTreeNode *> visited;
	visited.push_back(this);
	list.append(this);

	while(!list.empty()) {
		AcqTreeNode *current = list.front();
		list.pop_front();
		for(AcqTreeNode *child : qAsConst(current->m_treeChildren)) {
			if(!visited.contains(child)) {
				visited.push_back(child);
				list.push_back(child);
			}
		}
	}
	return visited;
}

QString AcqTreeNode::name() const { return m_name; }

AcqTreeNode *AcqTreeNode::treeParent() const { return m_treeParent; }

void AcqTreeNode::setTreeParent(AcqTreeNode *newTreeParent) { m_treeParent = newTreeParent; }

QList<AcqTreeNode *> AcqTreeNode::treeChildren() const { return m_treeChildren; }
