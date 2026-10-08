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

#include "block.h"
#include "datastore.h"

#include <atomic>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

namespace scopy {
namespace acq {

// Produces sample chunks into the DataStore, one per enabled channel per cycle.
//
// Lifecycle, all on the engine's worker thread: onStart() once, acquire() per
// cycle, onStop() once. Throwing from any of them aborts the run and surfaces
// as a Critical report.
//
// The channel map is shared with the GUI thread, so acquire() must go through
// enabledChannels() rather than touch it directly; the returned list is a
// snapshot valid for the rest of the cycle.
class SCOPY_CORE_EXPORT SourceBlock : public Block
{
	Q_OBJECT
public:
	explicit SourceBlock(const QString &id, QObject *parent = nullptr);

	virtual void acquire(DataStore *store) = 0;

	// False when this source's device is not present in the context it was
	// built against: absent, not failed. A source needing no device never
	// overrides this. Asked on the GUI thread right after construction, so a
	// host can build every source it knows of and keep the ones that answer
	// true — which means an implementation must be safe to call before
	// onStart() has run.
	virtual bool isAvailable() const { return true; }

	// Acquire and release device resources. Overrides must call the base.
	virtual void onStart();
	virtual void onStop();

	const QString &id() const { return name(); }

	void setBufferSize(std::size_t size);
	std::size_t bufferSize() const;

	void enableChannel(const QString &channelId, bool en);
	void removeChannel(const QString &channelId);
	void disableAllChannels();

	bool isChannelEnabled(const QString &channelId) const;
	QList<QString> enabledChannels() const;
	QList<QString> channelIds() const;

	// One raw key per registered channel — the convention every SourceBlock
	// subclass writes under. Sources that produce a derived stream instead
	// (e.g. MathSource) override this again.
	QList<DataKey> outputKeys() const override;

	// A Curve labelled with the channel id, for any of this source's raw keys.
	// A source producing something that is not a trace — a logic capture, a
	// spectrum — overrides this.
	std::optional<StreamInfo> streamInfo(const DataKey &key) const override;

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	// A channel was added or removed.
	void channelsChanged();
	// An existing channel's enable state flipped. Not emitted on first add.
	void channelEnabledChanged(const QString &channelId, bool en);

protected:
	std::size_t m_bufferSize{1024};
	std::atomic<bool> m_stopRequested{false};

private:
	mutable QMutex m_channelMutex;
	QMap<QString, bool> m_channels;
};

} // namespace acq
} // namespace scopy
