/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "scopy-core_export.h"

#include "datakey.h"
#include "samplebuffer.h"

#include <optional>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>

namespace scopy {
namespace acq {

// Central key/value store for acquired and derived sample streams. Every method is
// safe to call from any thread.
//
// History depth is not set directly: consumers register named claims (claimDepth)
// and a key's capacity is the max over its live claims, so a stream that is both
// plotted and decoded is never clipped by either. Sample claims are converted to
// chunks on every push, against the length chunks actually arrive in.
class SCOPY_CORE_EXPORT DataStore : public QObject
{
	Q_OBJECT
public:
	explicit DataStore(QObject *parent = nullptr);

	// --- Writing ---------------------------------------------------------

	void write(const DataKey &key, SampleVariant vec);

	// Replaces the key's whole history, for a producer whose output is a full buffer.
	// Depth claims are not applied: nothing is arriving.
	void copy(const DataKey &key, SampleBuffer buffer);

	// --- Reading ---------------------------------------------------------

	// Newest chunk. nullopt if the key is absent or has no chunks yet.
	// Cheaper than snapshot() — copies one chunk, not the whole history.
	std::optional<SampleVariant> latest(const DataKey &key) const;

	// Newest chunk narrowed to T. nullopt also when the stream holds a
	// different type, so callers get one check instead of three.
	template <class T>
	std::optional<T> latestAs(const DataKey &key) const
	{
		std::optional<SampleVariant> v = latest(key);
		if(!v || !std::holds_alternative<T>(*v))
			return std::nullopt;
		return std::get<T>(std::move(*v));
	}

	// Newest chunk converted to float, whatever the stream's numeric type (a 1-bit logic
	// line reads as 0.0/1.0). nullopt if absent, empty or annotations.
	std::optional<QVector<float>> latestFloat(const DataKey &key) const;

	// The newest `plotSize` samples, oldest-first, spanning as many chunks as
	// the stream's claimed depth allows. Preserves the stream's sample type;
	// windowFloat() converts, windowAs<T>() narrows.
	SampleVariant window(const DataKey &key, int plotSize) const;
	QVector<float> windowFloat(const DataKey &key, int plotSize) const;

	template <class T>
	T windowAs(const DataKey &key, int plotSize) const
	{
		SampleVariant v = window(key, plotSize);
		return std::holds_alternative<T>(v) ? std::get<T>(std::move(v)) : T{};
	}

	// Full history, for consumers that need every chunk (e.g. a waterfall).
	SampleBuffer snapshot(const DataKey &key) const;

	// --- Metadata --------------------------------------------------------

	bool contains(const DataKey &key) const;
	std::optional<SampleType> typeOf(const DataKey &key) const;
	std::size_t depth(const DataKey &key) const;
	QList<DataKey> keys() const;

	// Stream-level annotation descriptor. Set once by the producer (it is
	// constant across chunks) and read by consumers that need to interpret
	// the stream — notably a decoder taking these annotations as input.
	// Survives clear(); dropped by reset() and remove().
	void setAnnotationInfo(const DataKey &key, const AnnotationStreamInfo &info);
	std::optional<AnnotationStreamInfo> annotationInfo(const DataKey &key) const;

	// What a key means (label, unit, how to draw it) is not stored here; it is asked of
	// the producing block through AcquisitionEngine::streamInfo().

	// Monotonic count of write() calls. Sample it around an operation to tell
	// whether anything was written without copying key sets.
	quint64 writeCount() const;

	// --- History depth ---------------------------------------------------

	// One consumer's history requirement, kept in its own unit:
	//   samples(n) — "I draw a window n samples wide"; re-derived on every push.
	//   chunks(n)  — "I need n past chunks" (waterfall rows, averaged FFT frames).
	// `extraChunks` is added on top of a sample claim, for a reader that reads over-long
	// and re-anchors (a centred trigger window). Capacity is a max over claims, not a sum,
	// so this has to be part of the one claim.
	struct Claim
	{
		std::size_t amount{1};
		bool inSamples{false};
		std::size_t extraChunks{0};

		static Claim samples(std::size_t n, std::size_t extraChunks = 0) { return {n, true, extraChunks}; }
		static Claim chunks(std::size_t n) { return {n, false, 0}; }
	};

	// Register `claimant`'s requirement for `key`, replacing that claimant's
	// previous claim on that key whichever unit it was in. Effective capacity is
	// the max over claims.
	void claimDepth(const DataKey &key, const QString &claimant, Claim claim);
	void releaseDepth(const DataKey &key, const QString &claimant);
	// Drop every claim held by `claimant` across all keys.
	void releaseClaimant(const QString &claimant);

	// Chunks needed to cover `samples` arriving `bufferSize` at a time.
	static std::size_t depthForWindow(std::size_t samples, std::size_t bufferSize);

	// --- Lifecycle -------------------------------------------------------

	void clear();			 // drop chunks, keep keys and claims
	void reset();			 // drop everything
	void remove(const DataKey &key); // drop the key's data, keep its claims

	// Per-cycle dirty set: which keys were written since the last beginCycle().
	// Drives processor scheduling in AcquisitionEngine.
	void beginCycle();
	QSet<DataKey> cycleKeys() const;

Q_SIGNALS:
	// Emitted when a key is added or removed. Emitted from whichever thread
	// wrote, so GUI consumers must use Qt::QueuedConnection.
	void keysChanged(QList<DataKey> keys);

private:
	// Recomputes and applies capacity for `key`. Caller holds m_mutex.
	void applyDepthLocked(const DataKey &key);

	QMap<DataKey, SampleBuffer> m_data;
	QHash<DataKey, AnnotationStreamInfo> m_annInfo;
	QHash<DataKey, QHash<QString, Claim>> m_claims;
	QSet<DataKey> m_cycleKeys;
	quint64 m_writeCount{0};
	mutable QMutex m_mutex;
};

} // namespace acq
} // namespace scopy
