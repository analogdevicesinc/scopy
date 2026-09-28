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

#ifndef FILESOURCEBLOCK_H
#define FILESOURCEBLOCK_H

#include "csvfilereader.h"
#include "filereader.h"

#include <core/acq_engine/SourceBlock.h>

#include <memory>
#include <optional>
#include <vector>
#include <QMutex>

namespace scopy {
namespace adc {

// A source that reads files instead of a device.
//
// Structured like SnapshotSource: one block, N slots, added and removed from the block's own
// settings page. A slot is one file, so it carries N channels — their ids are "<slot>-<channel>",
// named after the file, which is what keeps two slots' streams apart.
//
// Cyclic mode hands out one chunk of the file per cycle, looping — a recording played back the way
// a live source writes. Off, the whole file is published at once, on demand: publish() pushes it
// into the store whenever something changes, so the data appears with the engine stopped and
// pressing Run is not needed to see it.
//
// acquire() republishes because every run starts with DataStore::clear().
//
// Non-cyclic with this as the only enabled source, a run has nothing to block on and the engine
// loop will spin on one core. Any other enabled source paces it, and cyclic mode is bounded by
// the chunks it hands out.
class FileSourceBlock : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	// What the panel draws one row from. A copy, so the caller never holds a reference into
	// m_slots — the reader is move-only and lives under the mutex.
	struct SlotInfo
	{
		QString title;
		QString path;
		int     channelCount{0};
		int     sampleCount{0};
		double  sampleRate{0.0};
		bool    declaresRate{false};
		bool    cyclic{false};
		int     playhead{0};

		std::optional<CsvFileReader::FirstColumnMode> firstColumnMode;
	};

	// `parent` should be the AcquisitionEngine: that is where the store this publishes into
	// comes from. A parentless block parses files but reaches no store.
	explicit FileSourceBlock(const QString &id = QStringLiteral("file"), QObject *parent = nullptr);
	~FileSourceBlock() override;

	int addSlot();
	void removeSlot(int index);
	QList<SlotInfo> slotInfos() const;

	// Parses and publishes. False on failure; the reason is reported.
	bool setSlotFile(int index, const QString &path);

	void setSlotCyclic(int index, bool on);

	// Used when the file declares no rate.
	void setSlotSampleRate(int index, double sr);
	void setSlotFirstColumnMode(int index, CsvFileReader::FirstColumnMode mode);

	void acquire(scopy::acq::DataStore *store) override;
	void onStart() override;

	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	// The store changed outside a cycle, so views must re-pull and repaint.
	void published();
	// A slot was added or removed; the panel rebuilds its rows.
	void slotsChanged();
	// That slot's file or mode changed; the panel resyncs its row.
	void slotChanged(int index);

private:
	struct Slot
	{
		QString                     title;
		QString                     path;
		std::unique_ptr<FileReader> reader;
		// "<title>-<channel>", the channel component of this slot's keys.
		QStringList channels;
		double      declaredRate{0.0};
		double      userRate{0.0};

		bool cyclic{false};
		int  playhead{0};
		bool publishedThisRun{false};

		int sampleCount() const;
		double rate() const { return declaredRate > 0.0 ? declaredRate : userRate; }
	};

	using Writes = QList<QPair<scopy::acq::DataKey, scopy::acq::SampleBuffer>>;
	using Chunks = QList<QPair<scopy::acq::DataKey, scopy::acq::SampleVariant>>;
	using Keys = QList<scopy::acq::DataKey>;

	void publish();
	void publishTo(scopy::acq::DataStore *store);

	// Caller holds m_mutex. Whole file for the enabled channels, and the keys of the disabled
	// ones to drop — the latter also in cyclic mode, where the enabled ones are acquire()'s.
	void collectWhole(const Slot &slot, Writes &writes, Keys &drops) const;
	// Caller holds m_mutex. One chunk per enabled channel from the playhead, which it advances.
	void collectChunk(Slot &slot, Chunks &chunks, Keys &drops) const;

	void apply(scopy::acq::DataStore *store, const Keys &drops, Writes &writes, Chunks &chunks) const;

	// Re-reads the slot's channel list off its reader, dropping keys the file no longer has.
	void adoptChannels(int index);
	// Drops the slot's keys, so a title, mode or file change leaves nothing stale behind.
	void dropSlotKeys(scopy::acq::DataStore *store, const QStringList &channels) const;
	// The file's base name, sanitised and made unique against the other slots.
	QString uniqueTitle(const QString &path, int exceptIndex) const;

	Slot *slotAt(int index);
	const Slot *slotAt(int index) const;

	mutable QMutex                     m_mutex;
	std::vector<std::unique_ptr<Slot>> m_slots;

	// The store to publish into outside a cycle — the engine's, taken from the parent at
	// construction. Borrowed, and set once: acquire() uses the store the engine passes in, not
	// this. Null without an engine parent, which every read here already guards.
	scopy::acq::DataStore *const m_store{nullptr};
};

} // namespace adc
} // namespace scopy

#endif // FILESOURCEBLOCK_H
