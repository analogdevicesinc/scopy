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

SampleVariant DataStore::readWindowNative(const DataKey &key, int plotSize)
{
	if(plotSize <= 0)
		return QVector<float>{};

	SampleBuffer snapshot;
	{
		QMutexLocker lk(&m_mutex);
		auto it = m_data.find(key);
		if(it == m_data.end())
			return QVector<float>{};
		if(it->type() == SampleType::Annotation)
			return QVector<Annotation>{};
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

QVector<float> DataStore::readWindow(const DataKey &key, int plotSize)
{
	SampleVariant v = readWindowNative(key, plotSize);
	return std::visit([](auto &&vec) -> QVector<float> {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<VecT, QVector<Annotation>>) {
			return {};
		} else if constexpr(std::is_same_v<VecT, QVector<float>>) {
			return std::forward<decltype(vec)>(vec);
		} else {
			QVector<float> out;
			out.resize(vec.size());
			float *dst = out.data();
			for(int i = 0; i < vec.size(); ++i)
				dst[i] = static_cast<float>(vec[i]);
			return out;
		}
	}, std::move(v));
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
	SampleVariant assembled = assembleWindow(snapshot, windowSize);
	if(!std::holds_alternative<QVector<quint8>>(assembled))
		return {};
	return std::get<QVector<quint8>>(std::move(assembled));
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

SampleVariant DataStore::assembleWindow(const SampleBuffer &buf, int plotSize)
{
	if(buf.empty() || plotSize <= 0)
		return QVector<float>{};

	return std::visit([&](const auto &firstVec) -> SampleVariant {
		using VecT = std::decay_t<decltype(firstVec)>;
		if constexpr(std::is_same_v<VecT, QVector<Annotation>>) {
			return QVector<Annotation>{};
		} else {
			const std::size_t depth = buf.depth();
			if(depth == 1) {
				if(firstVec.size() <= plotSize)
					return firstVec;
				return firstVec.mid(firstVec.size() - plotSize);
			}
			std::size_t total = 0;
			for(std::size_t i = 0; i < depth; ++i)
				total += static_cast<std::size_t>(
					std::get<VecT>(buf.sample(i)).size());
			VecT out;
			out.reserve(static_cast<int>(total));
			for(std::size_t i = depth; i-- > 0;) {
				const auto &v = std::get<VecT>(buf.sample(i));
				out.append(v);
			}
			if(out.size() > plotSize)
				out = out.mid(out.size() - plotSize);
			return out;
		}
	}, buf.sample(0));
}

QVector<float> DataStore::assembleWindowFloat(const SampleBuffer &buf, int plotSize)
{
	SampleVariant v = assembleWindow(buf, plotSize);
	return std::visit([](auto &&vec) -> QVector<float> {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<VecT, QVector<Annotation>>) {
			return {};
		} else if constexpr(std::is_same_v<VecT, QVector<float>>) {
			return std::forward<decltype(vec)>(vec);
		} else {
			QVector<float> out;
			out.resize(vec.size());
			float *dst = out.data();
			for(int i = 0; i < vec.size(); ++i)
				dst[i] = static_cast<float>(vec[i]);
			return out;
		}
	}, std::move(v));
}

} // namespace acq
} // namespace scopy
