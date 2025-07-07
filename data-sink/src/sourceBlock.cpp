#include <include/data-sink/sourceBlock.h>
using namespace scopy::datasink;

SourceBlock::SourceBlock(QString name)
	: BasicBlock(name)
	, m_size(0)
{
	QObject::connect(this, &SourceBlock::requestData, this, &SourceBlock::onRequestData, Qt::QueuedConnection);
}
SourceBlock::~SourceBlock() {}

QString SourceBlock::name() { return m_name; }

size_t SourceBlock::bufferSize() { return m_size; }

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
	m_size = size;
}

void SourceBlock::onRequestData()
{
	m_cancelRequested.store(false);
	QMutexLocker locker(&m_mutex); // Lock the mutex during data creation
	BlockData data = createData();

	if(m_size <= 0) {
		qDebug() << m_name << ": invalid buffer size: " << m_size << Qt::endl;
		return;
	}

	for(auto it = data.begin(); it != data.end(); ++it) {
		if(it.value().data.empty()) {
			qDebug() << m_name << ": empty data for channel " << it.key() << Qt::endl;
			continue;
		}

		Q_EMIT newData(it.value(), it.key());
	}
}
