// #include <QDebug>
// #include <include/data-sink/blockManager.h>
// #include <include/data-sink/proxyblock.h>

// using namespace scopy::datasink;

// // ProxyBlock Implementation
// ProxyBlock::ProxyBlock(QObject *parent)
// 	: QObject(parent)
// 	, m_enabled(false)
// 	, m_built(false)
// {
// }

// ProxyBlock::~ProxyBlock()
// {
// 	if (m_built) {
// 		// destroyFilterBlock();
// 	}
// }

// void ProxyBlock::setEnabled(bool v)
// {
// 	if (m_enabled != v) {
// 		m_enabled = v;
// 		emit requestRebuild();
// 	}
// }
