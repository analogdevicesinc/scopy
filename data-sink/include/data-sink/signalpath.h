// #ifndef SIGNALPATH_H
// #define SIGNALPATH_H

// #include "proxyblock.h"
// #include "scopy-data-sink_export.h"
// #include <QObject>

// /**
//  * Signal path class replacing GRSignalPath
//  * Manages a chain of ProxyBlocks for a single channel
//  */
// namespace scopy::datasink {
// class SCOPY_DATA_SINK_EXPORT SignalPath : public QObject
// {
// 	Q_OBJECT
// public:
// 	SignalPath(QString name, QObject *parent = nullptr);
// 	virtual ~SignalPath();

// 	void append(ProxyBlock *block);
// 	void clear();

// 	QString name() const { return m_name; }
// 	QList<ProxyBlock*> path() const { return m_blocks; }
// 	int blockCount() const { return m_blocks.size(); }

// 	void setEnabled(bool enabled);
// 	bool enabled() const { return m_enabled; }

// 	// Get first and last blocks for connection
// 	FilterBlock* getFirstBlock() const;
// 	FilterBlock* getLastBlock() const;

// protected:
// 	QList<ProxyBlock*> m_blocks;
// 	QString m_name;
// 	bool m_enabled;
// };
// }

// #endif // SIGNALPATH_H
