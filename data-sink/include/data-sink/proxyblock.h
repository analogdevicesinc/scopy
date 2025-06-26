// #ifndef PROXYBLOCK_H
// #define PROXYBLOCK_H

// #include <QObject>
// #include <QList>
// #include <QString>
// #include <QMutex>
// #include "filterBlock.h"
// #include "scopy-data-sink_export.h"

// // Forward declarations
// // class BlockManager;
// // class ChannelComponent;

// /**
//  * Base class replacing GRProxyBlock
//  * Manages a single filter block within the BlockManager system
//  */
// namespace scopy::datasink {
// class SCOPY_DATA_SINK_EXPORT ProxyBlock : public QObject
// {
// 	Q_OBJECT
// public:
// 	ProxyBlock(QObject *parent = nullptr);
// 	virtual ~ProxyBlock();

// 	void setEnabled(bool v);
// 	bool enabled() const { return m_enabled; }
// 	bool built() const { return m_built; }

// 	// Get the underlying filter block
// 	virtual FilterBlock* getFilterBlock() = 0;

// Q_SIGNALS:
// 	void requestRebuild();

// protected:
// 	virtual void createFilterBlock() = 0;
// 	virtual void destroyFilterBlock() = 0;

// 	bool m_enabled;
// 	bool m_built;
// };
// }

// #endif // PROXYBLOCK_H
