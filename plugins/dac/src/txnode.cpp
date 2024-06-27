#include "txnode.h"

using namespace scopy;

TxNode::TxNode(QString uuid, iio_channel *chn, QObject *parent)
	: QObject(parent)
	, m_channel(chn)
	, m_txUuid(uuid)
{

}

TxNode::~TxNode()
{
	// TBD
	// delete all child nodes
}

TxNode* TxNode::addChildNode(QString uuid, iio_channel *chn)
{
	TxNode *child = m_childNodes.value(uuid, nullptr);
	if (!child) {
		child = new TxNode(uuid, chn, this);
		m_childNodes.insert(uuid, child);
	}
	return child;
}

QMap<QString, TxNode *> TxNode::getTones() const
{
	return m_childNodes;
}

QString TxNode::getUuid() const
{
	return m_txUuid;
}

iio_channel *TxNode::getChannel()
{
	return m_channel;
}
