// #include <include/data-sink/signalpath.h>

// using namespace scopy::datasink;

// // SignalPath Implementation
// SignalPath::SignalPath(QString name, QObject *parent)
// 	: QObject(parent)
// 	, m_name(name)
// 	, m_enabled(false)
// {
// }

// SignalPath::~SignalPath()
// {
// 	clear();
// }

// void SignalPath::append(ProxyBlock *block)
// {
// 	if (block && !m_blocks.contains(block)) {
// 		m_blocks.append(block);
// 		block->setParent(this);
// 	}
// }

// void SignalPath::clear()
// {
// 	qDeleteAll(m_blocks);
// 	m_blocks.clear();
// }

// void SignalPath::setEnabled(bool enabled)
// {
// 	if (m_enabled != enabled) {
// 		m_enabled = enabled;

// 		// Enable/disable all blocks in the path
// 		for (ProxyBlock *block : m_blocks) {
// 			block->setEnabled(enabled);
// 		}
// 	}
// }

// FilterBlock* SignalPath::getFirstBlock() const
// {
// 	if (m_blocks.isEmpty()) {
// 		return nullptr;
// 	}
// 	return m_blocks.first()->getFilterBlock();
// }

// FilterBlock* SignalPath::getLastBlock() const
// {
// 	if (m_blocks.isEmpty()) {
// 		return nullptr;
// 	}
// 	return m_blocks.last()->getFilterBlock();
// }
