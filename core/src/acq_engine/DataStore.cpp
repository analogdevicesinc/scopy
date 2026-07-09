#include "DataStore.h"

#include <QMutexLocker>

namespace scopy {
namespace acq {

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

void DataStore::ensureHistoryDepth(const DataKey &key, std::size_t depth)
{
	QMutexLocker lk(&m_mutex);
	SampleBuffer &buf = m_data[key];
	if(buf.historySize() < depth)
		buf.setHistorySize(depth);
}

SampleBuffer DataStore::read(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	if(it == m_data.end())
		return SampleBuffer{};
	return *it;
}

QVector<float> DataStore::readWindow(const DataKey &key, int plotSize)
{
	if(plotSize <= 0)
		return {};

	SampleBuffer snapshot;
	{
		QMutexLocker lk(&m_mutex);
		auto it = m_data.find(key);
		if(it == m_data.end())
			return {};
		// Annotation buffers carry no numeric samples; assembleWindow
		// returns empty for them. Skip both the historySize mutation
		// and the copy so decoder-output keys are never mutated by
		// the display path.
		if(it->type() == SampleType::Annotation)
			return {};
		const std::size_t chunkSize = it->size();
		if(chunkSize > 0) {
			const std::size_t need = requiredHistoryDepth(
				static_cast<std::size_t>(plotSize), chunkSize);
			if(it->historySize() < need)
				it->setHistorySize(need);
		}
		snapshot = *it;
	}
	return assembleWindow(snapshot, plotSize);
}

QVector<quint8> DataStore::readWindowU8(const DataKey &key, int windowSize) const
{
	if(windowSize <= 0)
		return {};
	SampleBuffer snapshot;
	{
		QMutexLocker lk(&m_mutex);
		auto it = m_data.find(key);
		if(it == m_data.end())
			return {};
		if(it->type() != SampleType::UInt8)
			return {};
		snapshot = *it;
	}
	QVector<quint8> out;
	const std::size_t depth = snapshot.depth();
	for(std::size_t i = depth; i-- > 0;) {
		const auto &vec = std::get<QVector<quint8>>(snapshot.sample(i));
		out.reserve(out.size() + static_cast<int>(vec.size()));
		for(quint8 s : vec)
			out.append(s);
	}
	if(out.size() > windowSize)
		out = out.mid(out.size() - windowSize);
	return out;
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

void DataStore::remove(const DataKey &key)
{
	bool changed = false;
	{
		QMutexLocker lk(&m_mutex);
		changed = (m_data.remove(key) > 0);
		m_cycleKeys.remove(key);
	}
	if(changed)
		emit keysChanged(keys());
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

std::size_t DataStore::requiredHistoryDepth(std::size_t plotSize, std::size_t bufferSize)
{
	if(bufferSize == 0)
		return 1;
	const std::size_t d = (plotSize + bufferSize - 1) / bufferSize;
	return d < 1 ? 1 : d;
}

QVector<float> DataStore::assembleWindow(const SampleBuffer &buf, int plotSize)
{
	QVector<float> out;
	if(buf.empty() || plotSize <= 0)
		return out;

	const std::size_t depth = buf.depth();
	for(std::size_t i = depth; i-- > 0;) {
		std::visit(
			[&out](const auto &vec) {
				using VecT = std::decay_t<decltype(vec)>;
				if constexpr(std::is_same_v<VecT, QVector<Annotation>>) {
				} else {
					out.reserve(out.size() + static_cast<int>(vec.size()));
					for(const auto &s : vec)
						out.append(static_cast<float>(s));
				}
			},
			buf.sample(i));
	}

	if(out.size() > plotSize)
		out = out.mid(out.size() - plotSize);
	return out;
}

} // namespace acq
} // namespace scopy
