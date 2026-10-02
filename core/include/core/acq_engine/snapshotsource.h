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
#include "sourceblock.h"

#include <optional>
#include <QList>
#include <QMutex>
#include <QString>

namespace scopy {
namespace acq {

// A source that acquires nothing: it freezes streams that already exist.
//
// A slot points at one DataStore key; capture() copies that key's SampleBuffer and the block
// republishes the copy under a name of its own — a reference curve next to the live trace.
// The title is the channel component of "<blockId>_<title>_snapshot" and is mandatory.
//
// Cyclic mode hands out one captured chunk per cycle, looping — a recording played back the
// way a live source writes. Off, the whole history is copied out at once.
//
// acquire() republishes every cycle because every run starts with DataStore::clear().
//
// Threading: capture() and the setters are GUI-thread, acquire() is the worker's. m_slots is
// guarded by m_mutex; every DataStore call happens with that lock released.
class SCOPY_CORE_EXPORT SnapshotSource : public SourceBlock
{
	Q_OBJECT
public:
	struct Slot
	{
		QString title; // channel component of the output key
		DataKey source;

		SampleBuffer data; // the frozen copy, empty until the first capture
		bool captured{false};

		// The source's descriptor at capture time, held so the snapshot keeps its
		// labelling after the producer re-declares.
		QString capturedFrom;
		QString unit;
		double sampleRate{0.0};
		std::optional<AnnotationStreamInfo> annInfo;

		bool cyclic{true};
		// Playback position in recording order — 0 is the *oldest* chunk, the opposite of
		// SampleBuffer's indexing. chunkAt() flips it.
		int playhead{0};

		int chunkCount() const { return static_cast<int>(data.depth()); }
		int sampleCount() const { return static_cast<int>(data.size()); }

		const SampleVariant &chunkAt(int pos) const
		{
			const int n = chunkCount();
			if(pos < 0 || pos >= n) {
				return data.sample(data.depth()); // out of range: empty chunk
			}
			return data.sample(static_cast<std::size_t>(n - 1 - pos));
		}
	};

	// `parent` should be the AcquisitionEngine: that is where the store to capture from comes
	// from. A parentless block captures nothing.
	explicit SnapshotSource(const QString &id, QObject *parent = nullptr);

	int addSlot(); // returns the new slot's index
	void removeSlot(int index);

	// Copies, so callers never hold a reference into m_slots.
	QList<Slot> allSlots() const;
	std::optional<Slot> slotAt(int index) const;

	void setSlotSource(int index, const DataKey &key);
	void setSlotCyclic(int index, bool on);

	// Rejects an empty title or one another slot holds — that would be two producers of one
	// key. exceptIndex -1 checks every slot.
	bool setSlotTitle(int index, const QString &title);
	bool isTitleAvailable(const QString &title, int exceptIndex = -1) const;

	void capture(int index);
	void clearSlot(int index);

	void acquire(DataStore *store) override;
	void onStart() override; // rewinds every playback position

	QList<DataKey> outputKeys() const override;
	std::optional<StreamInfo> streamInfo(const DataKey &key) const override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	// <sourceId>_<title>_snapshot, empty for an empty title. Single-segment stage because
	// DataKey parsing is right-anchored — which is also why sanitizeTitle() exists.
	static DataKey keyFor(const QString &sourceId, const QString &title);
	static QString sanitizeTitle(const QString &title);

Q_SIGNALS:
	void slotsChanged();	      // a slot was added or removed; rows rebuild
	void slotCaptured(int index); // that slot's buffer or mode changed

private:
	// Caller must not hold m_mutex: DataStore emits keysChanged, whose GUI handler calls
	// back in here.
	void dropKey(DataStore *store, const QString &title) const;

	mutable QMutex m_mutex;
	QList<Slot> m_slots;
	// Set once from the engine parent at construction. Raw and unguarded, unlike the widget's
	// pointers: only ever read on the GUI thread by capture() and the setters, which the
	// reader drives while the instrument is alive. acquire() uses the store the engine passes
	// in, not this.
	DataStore *const m_srcStore{nullptr};
};

} // namespace acq
} // namespace scopy
