#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "filterBlock.h"
#include "scopy-data-sink_export.h"
#include "sourceBlock.h"
#include <QThread>
#include <toolcomponent.h>

namespace scopy::datasink {
class OutputConnection
{
public:
	OutputConnection(BasicBlock *block, uint ch, QMetaObject::Connection conn)
	{
		final_block = block;
		final_block_ch = ch;
		connection = conn;
	}

	~OutputConnection() { QObject::disconnect(connection); }

	BasicBlock *final_block;
	uint final_block_ch;
	QMetaObject::Connection connection;
};

class SCOPY_DATA_SINK_EXPORT SourceBlockLink : public QObject
{
	Q_OBJECT
public:
	SourceBlockLink(SourceBlock *sourceBlock, bool threaded = false);
	~SourceBlockLink();

	SourceBlock *source;
	QThread *thread;
	QMap<uint, QSet<uint>> connected_outputs; // QMap<output_ch, QSet<connected_source_ch_list>>
	int finished_outputs;
	int active_outputs;

	uint enabledSourceChCount();
	void addSource(uint out_ch, uint source_ch);
	void updateActiveOutputs();
	void removeSource(uint out_ch, uint source_ch);
};

class SCOPY_DATA_SINK_EXPORT BlockManager : public QObject, public DataProvider
{
	Q_OBJECT
public:
	BlockManager(QString name, bool waitforAllSources = true);
	~BlockManager();

	/*
	 *  source: source block creating the data
	 *  source_ch: source block's conected ch id
	 *  final: final output block (may be the same as the source block)
	 *  final_ch: final block's connected ch id
	 *  out_ch: new ch id used for the manager's output
	 *  threaded: execute all block linked to the source in a thread
	 */
	void addLink(SourceBlock *source, uint source_ch, BasicBlock *final, uint final_ch, uint out_ch, bool threaded);
	void removeLink(SourceBlock *source, uint source_ch, uint out_ch);
	void setTargetFPS(uint fps); // set to 0 for inf fps target
	void setSingleShot(bool single);
	void setBufferSize(size_t size);
	QString name();

	static void connectBlockToFilter(BasicBlock *block, uint block_ch, uint filter_ch, FilterBlock *filter);
	static void disconnectBlockToFilter(BasicBlock *block, uint block_ch, uint filter_ch, FilterBlock *filter);

public Q_SLOTS:
	bool start();
	void stop();

private:
	// avoids connecting same output multiple times
	int addOutputLink(SourceBlock *source, BasicBlock *final, uint final_ch, uint out_ch);
	void emitOutputData(SourceBlock *source = nullptr);

Q_SIGNALS:
	void newData(ChannelDataVector *data, uint ch);
	void sentAllData();
	void doDisconnectBlockToFilter();
	void requestSingleShot(bool);
	void requestBufferSize(uint32_t);
	void started();
	void stopped();

private:
	QString m_name;
	bool m_running = false;
	bool m_singleShot = false;
	uint m_fps;
	bool m_waitforAllSources;
	QElapsedTimer *m_fpsTimer;
	qint64 m_fpsTimeElapsed;
	QThread *m_thread;
	QMap<SourceBlock *, SourceBlockLink *> m_blockLinks;
	size_t m_globalBufferSize;

	// QMap<output_ch, QPair<final_block, final_ch>> only used to avoid connecting requestData signal to
	// sourceBlocks multiple times
	QMap<uint, OutputConnection *> m_requestDataConnections;

	// DataProvider interface
public:
	size_t updateData() { return 0; };
	bool finished() { return true; };
	void setData(bool copy) {};
};
} // namespace scopy::datasink

#endif
