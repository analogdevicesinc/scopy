#ifndef SOURCEBLOCK_H
#define SOURCEBLOCK_H

#include "basicBlock.h"
#include "scopy-data-sink_export.h"
#include <QMap>
#include <QObject>
#include <QLoggingCategory>
#include <QMutex>

namespace scopy::datasink {
class SCOPY_DATA_SINK_EXPORT SourceBlock : public BasicBlock
{
	Q_OBJECT
public:
	SourceBlock(QString name = QString());
	~SourceBlock();
	QString name();
	size_t bufferSize();
	size_t plotSize();

public Q_SLOTS:
	virtual void enChannel(bool en, uint id);
	virtual bool isChannelEn(uint id);
	virtual void setBufferSize(size_t size);
	virtual void setPlotSize(size_t size);
	void onRequestData();

private:
	QMutex m_mutex;
	std::atomic_bool m_cancelRequested = false;
	virtual BlockData createData() = 0; // <channel, buffer>

Q_SIGNALS:
	void plotSizeChanged(size_t size);
	void requestData();
	void toggledCh(uint ch, bool en);
	void resetPlotBuffer();

protected:
	size_t m_bufferSize;
	size_t m_plotSize;
	QMap<uint, bool> m_channels; // <channel, enabled>
};
} // namespace scopy::datasink

#endif
