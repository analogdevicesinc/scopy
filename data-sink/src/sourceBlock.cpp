#include <include/data-sink/sourceBlock.h>
#include <iostream>
using namespace scopy::datasink;

SourceBlock::SourceBlock(QString name)
	: BasicBlock(name)
	, m_bufferSize(0)
	, m_plotSize(m_bufferSize)
{
	QObject::connect(this, &SourceBlock::requestData, this, &SourceBlock::onRequestData, Qt::QueuedConnection);
}
SourceBlock::~SourceBlock() {}

QString SourceBlock::name() { return m_name; }

size_t SourceBlock::bufferSize() { return m_bufferSize; }

size_t SourceBlock::plotSize() { return m_plotSize; }

void SourceBlock::enChannel(bool en, uint id)
{
	m_cancelRequested.store(true);
	QMutexLocker locker(&m_mutex); // Lock during modification
	if(!m_channels.contains(id)) {
		m_channels.insert(id, en);
		Q_EMIT toggledCh(id, en);
	} else if(m_channels.value(id) != en) {
		m_channels[id] = en;
		Q_EMIT toggledCh(id, en);
	}
}

bool SourceBlock::isChannelEn(uint id)
{
	QMutexLocker locker(&m_mutex);
	return m_channels[id];
}

void SourceBlock::setBufferSize(size_t size)
{
	m_cancelRequested.store(true);
	QMutexLocker locker(&m_mutex); // Lock during modification
	m_bufferSize = size;
}

void SourceBlock::setPlotSize(size_t size)
{
	Q_EMIT plotSizeChanged(size);
	m_plotSize = size;
}

void SourceBlock::onRequestData()
{
	m_cancelRequested.store(false);
	QMutexLocker locker(&m_mutex); // Lock the mutex during data creation

	// uint aqcCounter = (m_plotSize + m_bufferSize - 1) / m_bufferSize;
	// if(aqcCounter > 1) {
	// 	disconnect(this, &SourceBlock::requestData, this, &SourceBlock::onRequestData);
	// }

	// for(int i = 1; i <= aqcCounter; i++) {
		// if(i ==  aqcCounter) {
		// 	QObject::connect(this, &SourceBlock::requestData, this, &SourceBlock::onRequestData, Qt::QueuedConnection);
		// }

		BlockData data = createData();

		if(m_bufferSize <= 0) {
			qDebug() << m_name << ": invalid buffer size: " << m_bufferSize << Qt::endl;
			return;
		}

		for(auto it = data.begin(); it != data.end(); ++it) {
			if(it.value().data.empty()) {
				qDebug() << m_name << ": empty data for channel " << it.key() << Qt::endl;
				continue;
			}

			// std::cout << "newData ch " << it.key() << "       of size " << it.value().data.size() << "         counter " << i << std::endl;
			Q_EMIT newData(it.value(), it.key());
		}
	// }
}
