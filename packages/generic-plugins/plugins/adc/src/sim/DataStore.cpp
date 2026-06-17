#include "DataStore.h"

#include <QMutexLocker>

namespace scopy {
namespace adc {
namespace sim {

DataStore::DataStore(QObject *parent)
	: QObject(parent)
{}

void DataStore::write(const DataKey &key, SampleVariant vec)
{
	bool newKey;
	{
		QMutexLocker lk(&m_mutex);
		newKey = !m_data.contains(key);
		m_data[key].push(std::move(vec));
		m_cycleKeys.insert(key);
	}
	if(newKey)
		emit keysChanged(keys());
	emit dataWritten(key);
}

void DataStore::setHistorySize(const DataKey &key, std::size_t n)
{
	QMutexLocker lk(&m_mutex);
	m_data[key].setHistorySize(n);
}

SampleBuffer DataStore::read(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	if(it == m_data.end())
		return SampleBuffer{};
	return *it;
}

bool DataStore::contains(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	return m_data.contains(key);
}

QList<DataKey> DataStore::keys() const
{
	QMutexLocker lk(&m_mutex);
	return m_data.keys();
}

void DataStore::clear()
{
	QMutexLocker lk(&m_mutex);
	for(auto &buf : m_data) {
		std::size_t sz = buf.historySize();
		buf = SampleBuffer{};
		buf.setHistorySize(sz);
	}
}

void DataStore::reset()
{
	QMutexLocker lk(&m_mutex);
	m_data.clear();
}

void DataStore::beginCycle()
{
	QMutexLocker lk(&m_mutex);
	m_cycleKeys.clear();
}

QSet<DataKey> DataStore::cycleKeys() const
{
	QMutexLocker lk(&m_mutex);
	return m_cycleKeys;
}

} // namespace sim
} // namespace adc
} // namespace scopy
