/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "core/result.h"
#include "component/channel.h"

#include <QList>
#include <QObject>
#include <cstddef>
#include <qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Direction-neutral open() configuration. enabledChannels is the single source of
// truth for channel selection: Channel::streamIndex values of members of the stream
// (Stream::channels()). cyclic/kernelBuffers are leaf setters, not here.
struct StreamConfig
{
	QList<int> enabledChannels;
	size_t samplesCount = 4096;
};

// Streaming session + lifecycle. Direction-specific I/O lives on InputStream /
// OutputStream. open() takes the same neutral config in both directions.
// Member channels (sorted by streamIndex) are registered by the backend builder;
// they are siblings of the stream (children of the Device), not its children.
// The read/write StreamFormat::channels list the enabled members in this order.
//
// Channel selection: the backend may not support every subset of its members
// (e.g. an ADC that only streams all channels together). resolveEnabledChannels()
// maps a request to what will actually be enabled; openAsync() applies it and
// enabledChannels() reports the result. A consumer that asked for fewer channels
// than were enabled picks its entries by ChannelFormat::streamIndex.
class COMPONENT_EXPORT Stream : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool isOpen READ isOpen NOTIFY openedChanged)
	Q_PROPERTY(unsigned bufferIndex READ bufferIndex CONSTANT)
	Q_PROPERTY(QList<int> enabledChannels READ enabledChannels NOTIFY enabledChannelsChanged)
	Q_PROPERTY(quint64 samplesCount READ samplesCount CONSTANT)
public:
	explicit Stream(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~Stream() override = default;

	// Implementations must copy cfg before their first suspension: callers may pass a temporary.
	// They must resolve cfg.enabledChannels (resolveForOpen) and fail with -EINVAL when
	// nothing would be enabled, then record the applied config with commitConfig().
	virtual QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) = 0;
	virtual QCoro::Task<CommandResponse<void>> closeAsync() = 0;

	// Map a requested selection (streamIndex values) to the one the backend will enable:
	// members only, sorted, de-duplicated, widened/narrowed to backend constraints.
	// Empty means nothing can be enabled. Default: channels are independent.
	Q_INVOKABLE virtual QList<int> resolveEnabledChannels(const QList<int> &requested) const;

	// Change the selection. Closed: only records the resolved selection (the next
	// openAsync still takes its own config). Open: closes, then reopens with the
	// resolved selection and the same samplesCount; an empty resolution leaves it closed.
	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> setEnabledChannelsAsync(const QList<int> &channels);

	bool isOpen() const { return m_open; }
	// Which device buffer this stream drives. Distinguishes sibling streams of the
	// same direction on a multi-buffer device (libiio v1). 0 for single-buffer.
	unsigned bufferIndex() const { return m_bufferIndex; }
	const StreamConfig &config() const { return m_config; }
	QList<int> enabledChannels() const { return m_config.enabledChannels; }
	size_t samplesCount() const { return m_config.samplesCount; }

	// Streamable channels of this stream, sorted by streamIndex.
	Q_INVOKABLE QList<scopy::component::Channel *> channels() const { return m_channels; }
	// The member channel with this streamIndex, or nullptr.
	Channel *channelAt(long streamIndex) const;
	bool isChannelEnabled(long streamIndex) const { return m_config.enabledChannels.contains(int(streamIndex)); }
	// Builder-only: register a member channel (must carry a streamIndex >= 0).
	void addChannel(Channel *channel);

Q_SIGNALS:
	void streamError(const scopy::Error &error);
	void openedChanged(bool open);
	void enabledChannelsChanged(const QList<int> &channels);

protected:
	// For openAsync implementations: the resolved selection, or -EINVAL if empty.
	Result<QList<int>> resolveForOpen(const QList<int> &requested) const;
	// Record the applied config; emits enabledChannelsChanged when the selection changed.
	void commitConfig(const StreamConfig &cfg);

	QList<Channel *> m_channels;
	StreamConfig m_config;
	bool m_open = false;
	unsigned m_bufferIndex = 0;
};

// Discovery helpers. A device may hold several same-direction streams (one per
// device buffer on libiio v1), so callers must not assume uniqueness: enumerate
// with streamsOf<T>() and disambiguate by bufferIndex, or fetch one directly with
// streamAt<T>(). T is InputStream / OutputStream (direction) or a concrete leaf.
template <typename T>
QList<T *> streamsOf(const QObject *device)
{
	return device ? device->findChildren<T *>(QString(), Qt::FindDirectChildrenOnly) : QList<T *>{};
}

template <typename T>
T *streamAt(const QObject *device, unsigned bufferIndex = 0)
{
	for(T *s : streamsOf<T>(device)) {
		if(s->bufferIndex() == bufferIndex) {
			return s;
		}
	}
	return nullptr;
}

} // namespace scopy::component
