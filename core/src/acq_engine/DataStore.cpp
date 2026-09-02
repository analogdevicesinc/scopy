#include "DataStore.h"

#include <algorithm>
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
		// After the push, not before: capacity is derived from the chunk length, so
		// there has to be a chunk to measure. This is also what applies the claims
		// registered before the key existed — a view creates its channel (and claim)
		// ahead of the first cycle, and that is what makes the first window
		// full-depth rather than a single chunk.
		//
		// Every push, not just the first: the chunk length can change between cycles
		// (the acquisition buffer is resized), and a sample claim has to follow it
		// down as well as up. Cheap — a handful of claims and no allocation.
		applyDepthLocked(key);
		m_cycleKeys.insert(key);
		++m_writeCount;
	}
	if(newKey)
		Q_EMIT keysChanged(keys());
}

std::optional<SampleVariant> DataStore::latest(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	if(it == m_data.end() || it->empty())
		return std::nullopt;
	return it->sample(0);
}

SampleVariant DataStore::window(const DataKey &key, int plotSize) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	if(it == m_data.end())
		return QVector<float>{};
	return it->window(plotSize);
}

QVector<float> DataStore::windowFloat(const DataKey &key, int plotSize) const
{
	return toFloat(window(key, plotSize));
}

SampleBuffer DataStore::snapshot(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	return it == m_data.end() ? SampleBuffer{} : *it;
}

bool DataStore::contains(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	return m_data.contains(key);
}

std::optional<SampleType> DataStore::typeOf(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	return it == m_data.end() ? std::nullopt : it->type();
}

std::size_t DataStore::depth(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_data.find(key);
	return it == m_data.end() ? 0 : it->depth();
}

QList<DataKey> DataStore::keys() const
{
	QMutexLocker lk(&m_mutex);
	return m_data.keys();
}

quint64 DataStore::writeCount() const
{
	QMutexLocker lk(&m_mutex);
	return m_writeCount;
}

void DataStore::setAnnotationInfo(const DataKey &key, const AnnotationStreamInfo &info)
{
	QMutexLocker lk(&m_mutex);
	m_annInfo.insert(key, info);
}

std::optional<AnnotationStreamInfo> DataStore::annotationInfo(const DataKey &key) const
{
	QMutexLocker lk(&m_mutex);
	auto it = m_annInfo.constFind(key);
	if(it == m_annInfo.constEnd())
		return std::nullopt;
	return *it;
}

void DataStore::claimLocked(const DataKey &key, const QString &claimant, Claim c)
{
	c.amount = std::max<std::size_t>(1, c.amount);
	// One entry per claimant, so a claim in either unit replaces whatever that
	// claimant asked for before — including a claim in the other unit.
	m_claims[key][claimant] = c;
	applyDepthLocked(key);
}

void DataStore::claimSamples(const DataKey &key, const QString &claimant, std::size_t samples,
			     std::size_t extraChunks)
{
	QMutexLocker lk(&m_mutex);
	claimLocked(key, claimant, Claim{samples, /*inSamples=*/true, extraChunks});
}

void DataStore::claimChunks(const DataKey &key, const QString &claimant, std::size_t chunks)
{
	QMutexLocker lk(&m_mutex);
	claimLocked(key, claimant, Claim{chunks, /*inSamples=*/false, /*extraChunks=*/0});
}

void DataStore::releaseDepth(const DataKey &key, const QString &claimant)
{
	QMutexLocker lk(&m_mutex);
	auto it = m_claims.find(key);
	if(it == m_claims.end())
		return;
	it->remove(claimant);
	if(it->isEmpty())
		m_claims.erase(it);
	applyDepthLocked(key);
}

void DataStore::releaseClaimant(const QString &claimant)
{
	QMutexLocker lk(&m_mutex);
	for(auto it = m_claims.begin(); it != m_claims.end();) {
		const DataKey key = it.key();
		it->remove(claimant);
		if(it->isEmpty())
			it = m_claims.erase(it);
		else
			++it;
		applyDepthLocked(key);
	}
}

void DataStore::applyDepthLocked(const DataKey &key)
{
	auto buf = m_data.find(key);
	if(buf == m_data.end())
		return;

	// The length chunks are actually arriving in. Zero until the first push, which
	// is why a sample claim cannot be resolved before then — write() re-applies once
	// there is a chunk to measure.
	const std::size_t chunkLen = buf->size();

	std::size_t want = 1;
	auto claims = m_claims.constFind(key);
	if(claims != m_claims.constEnd())
		for(const Claim &c : *claims) {
			// extraChunks is part of the one claim rather than a claim of its own,
			// because it is a sum with the converted count and capacity is a max.
			const std::size_t n =
				c.inSamples ? depthForWindow(c.amount, chunkLen) + c.extraChunks : c.amount;
			want = std::max(want, n);
		}

	buf->setCapacity(want);
}

void DataStore::clear()
{
	QMutexLocker lk(&m_mutex);
	for(SampleBuffer &buf : m_data)
		buf.clearChunks();
	m_cycleKeys.clear();
}

void DataStore::reset()
{
	bool changed;
	{
		QMutexLocker lk(&m_mutex);
		changed = !m_data.isEmpty();
		m_data.clear();
		m_annInfo.clear();
		m_claims.clear();
		m_cycleKeys.clear();
	}
	if(changed)
		Q_EMIT keysChanged(keys());
}

void DataStore::remove(const DataKey &key)
{
	bool changed;
	{
		QMutexLocker lk(&m_mutex);
		changed = m_data.remove(key) > 0;
		m_annInfo.remove(key);
		m_claims.remove(key);
		m_cycleKeys.remove(key);
	}
	if(changed)
		Q_EMIT keysChanged(keys());
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

std::size_t DataStore::depthForWindow(std::size_t plotSize, std::size_t bufferSize)
{
	if(bufferSize == 0)
		return 1;
	return std::max<std::size_t>(1, (plotSize + bufferSize - 1) / bufferSize);
}

} // namespace acq
} // namespace scopy

#include "moc_DataStore.cpp"
