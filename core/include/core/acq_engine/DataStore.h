#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "SampleBuffer.h"

#include <optional>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>

namespace scopy {
namespace acq {

// Central key/value store for acquired and derived sample streams.
//
// Sources and processors write chunks; the engine and GUI read them. Every
// method is safe to call from any thread.
//
// History depth is not set directly. Consumers register a named claim — in
// samples (claimSamples) or in chunks (claimChunks), whichever their requirement
// is actually expressed in — and the effective capacity of a stream is the
// maximum over its live claims. This keeps independent consumers of the same key
// — a plot window, a waterfall, a decoder — from overwriting each other's
// requirements. Reads never mutate capacity.
//
// The samples->chunks conversion lives here and nowhere else. It needs the chunk
// length, and the store is the only place that knows the length chunks *actually*
// arrive in rather than the length some UI control last asked for; a consumer
// holding its own copy of the buffer size is how a claim outlives a change to it
// and pins history at the old ratio. Capacity is therefore recomputed on every
// push, so a sample claim tracks the data instead of a setting.
class SCOPY_CORE_EXPORT DataStore : public QObject
{
	Q_OBJECT
public:
	explicit DataStore(QObject *parent = nullptr);

	// --- Writing ---------------------------------------------------------

	void write(const DataKey &key, SampleVariant vec);

	// Replaces everything under `key` with a complete history, for a producer whose output
	// *is* a whole buffer rather than a chunk. Unlike write(), does not apply the key's
	// depth claims: a claim says how much history to keep as chunks arrive, and nothing is
	// arriving here.
	void copy(const DataKey &key, SampleBuffer buffer);

	// --- Reading ---------------------------------------------------------

	// Newest chunk. nullopt if the key is absent or has no chunks yet.
	// Cheaper than snapshot() — copies one chunk, not the whole history.
	std::optional<SampleVariant> latest(const DataKey &key) const;

	// Newest chunk narrowed to T. nullopt also when the stream holds a
	// different type, so callers get one check instead of three.
	template<class T>
	std::optional<T> latestAs(const DataKey &key) const
	{
		std::optional<SampleVariant> v = latest(key);
		if(!v || !std::holds_alternative<T>(*v))
			return std::nullopt;
		return std::get<T>(std::move(*v));
	}

	// The newest `plotSize` samples, oldest-first, spanning as many chunks as
	// the stream's claimed depth allows. Preserves the stream's sample type;
	// windowFloat() converts, windowAs<T>() narrows.
	SampleVariant  window(const DataKey &key, int plotSize) const;
	QVector<float> windowFloat(const DataKey &key, int plotSize) const;

	template<class T>
	T windowAs(const DataKey &key, int plotSize) const
	{
		SampleVariant v = window(key, plotSize);
		return std::holds_alternative<T>(v) ? std::get<T>(std::move(v)) : T{};
	}

	// Full history, for consumers that need every chunk (e.g. a waterfall).
	SampleBuffer snapshot(const DataKey &key) const;

	// --- Metadata --------------------------------------------------------

	bool                      contains(const DataKey &key) const;
	std::optional<SampleType> typeOf(const DataKey &key) const;
	std::size_t               depth(const DataKey &key) const;
	QList<DataKey>            keys() const;

	// Stream-level annotation descriptor. Set once by the producer (it is
	// constant across chunks) and read by consumers that need to interpret
	// the stream — notably a decoder taking these annotations as input.
	// Survives clear(); dropped by reset() and remove().
	void setAnnotationInfo(const DataKey &key, const AnnotationStreamInfo &info);
	std::optional<AnnotationStreamInfo> annotationInfo(const DataKey &key) const;

	// No stream presentation descriptor here — this store holds samples. What a
	// key *means* (label, unit, rate, X source, how to draw it) is owned by the
	// block that produces it and asked for through
	// AcquisitionEngine::streamInfo(), so there is no second copy to keep in sync
	// with a producer whose declaration changed.

	// Monotonic count of write() calls. Sample it around an operation to tell
	// whether anything was written without copying key sets.
	quint64 writeCount() const;

	// --- History depth ---------------------------------------------------

	// Register `claimant`'s requirement for `key`; either call replaces that
	// claimant's previous claim on that key, whichever unit it was in. Effective
	// capacity is the max over claims.
	//
	// Pick the one your requirement is naturally in and let the store do the rest:
	//   claimSamples — "I draw a window this many samples wide". The store keeps
	//     enough chunks to cover it at the length chunks are currently arriving in,
	//     and re-derives that on every push, so the claim needs no refreshing when
	//     the acquisition buffer changes size.
	//   claimChunks — "I need this many past chunks", where a chunk is the unit
	//     itself: one waterfall row, one averaged FFT frame, one annotation set.
	//     Independent of the chunk length, so nothing to convert.
	//
	// Claiming samples for a chunk-shaped requirement is the bug this split exists
	// to prevent: it makes the claim scale with the chunk length, which is exactly
	// what it should not do.
	//
	// `extraChunks` is added on top of the converted count, for a reader whose
	// requirement genuinely is both: a centred trigger window is read over-long
	// (plotSize + up to one chunk) and then re-anchored, so it needs the window
	// *plus* a chunk. Two separate claimants could not express that — capacity is
	// the max over claims, never a sum.
	void claimSamples(const DataKey &key, const QString &claimant, std::size_t samples,
			  std::size_t extraChunks = 0);
	void claimChunks(const DataKey &key, const QString &claimant, std::size_t chunks);
	void releaseDepth(const DataKey &key, const QString &claimant);
	// Drop every claim held by `claimant` across all keys.
	void releaseClaimant(const QString &claimant);

	// Chunks needed to cover `samples` arriving `bufferSize` at a time. Public for
	// the one caller that genuinely needs a chunk *count* rather than a claim:
	// AcquisitionEngine::single(), which runs a fixed number of cycles.
	static std::size_t depthForWindow(std::size_t samples, std::size_t bufferSize);

	// --- Lifecycle -------------------------------------------------------

	void clear();  // drop chunks, keep keys and claims
	void reset();  // drop everything
	void remove(const DataKey &key);

	// Per-cycle dirty set: which keys were written since the last beginCycle().
	// Drives processor scheduling in AcquisitionEngine.
	void          beginCycle();
	QSet<DataKey> cycleKeys() const;

Q_SIGNALS:
	// Emitted when a key is added or removed. Emitted from whichever thread
	// wrote, so GUI consumers must use Qt::QueuedConnection.
	void keysChanged(QList<DataKey> keys);

private:
	// One claimant's requirement, in the unit it was expressed in. Kept in its
	// original unit rather than converted at claim time: a sample claim has to be
	// re-derived whenever the chunk length changes, which is the whole point.
	struct Claim
	{
		std::size_t amount{1};
		bool        inSamples{false};
		// Added after the conversion, and only meaningful for a sample claim. See
		// claimSamples.
		std::size_t extraChunks{0};
	};

	void claimLocked(const DataKey &key, const QString &claimant, Claim c);
	// Recomputes and applies capacity for `key`. Caller holds m_mutex.
	void applyDepthLocked(const DataKey &key);

	QMap<DataKey, SampleBuffer>              m_data;
	QHash<DataKey, AnnotationStreamInfo>     m_annInfo;
	QHash<DataKey, QHash<QString, Claim>>    m_claims;
	QSet<DataKey>                              m_cycleKeys;
	quint64                                    m_writeCount{0};
	mutable QMutex                             m_mutex;
};

} // namespace acq
} // namespace scopy
