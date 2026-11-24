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

#include "component/stream.h"

#include <algorithm>
#include <cerrno>
#include <QUuid>

using namespace scopy;
using namespace scopy::component;

Channel *Stream::channelAt(long streamIndex) const
{
	for(Channel *ch : m_channels) {
		if(ch->streamIndex() == streamIndex) {
			return ch;
		}
	}
	return nullptr;
}

void Stream::addChannel(Channel *channel)
{
	if(!channel || channel->streamIndex() < 0 || m_channels.contains(channel)) {
		return;
	}
	auto it =
		std::upper_bound(m_channels.begin(), m_channels.end(), channel, [](const Channel *a, const Channel *b) {
			return a->streamIndex() < b->streamIndex();
		});
	m_channels.insert(it, channel);
}

QList<int> Stream::resolveEnabledChannels(const QList<int> &requested) const
{
	QList<int> resolved;
	for(const Channel *ch : m_channels) {
		const int idx = static_cast<int>(ch->streamIndex());
		if(requested.contains(idx) && !resolved.contains(idx)) {
			resolved.append(idx);
		}
	}
	return resolved;
}

Result<QList<int>> Stream::resolveForOpen(const QList<int> &requested) const
{
	const QList<int> resolved = resolveEnabledChannels(requested);
	if(resolved.isEmpty()) {
		return Unexpected{Error{-EINVAL, QStringLiteral("no channel enabled")}};
	}
	return resolved;
}

void Stream::commitConfig(const StreamConfig &cfg)
{
	const bool changed = cfg.enabledChannels != m_config.enabledChannels;
	m_config = cfg;
	if(changed) {
		Q_EMIT enabledChannelsChanged(m_config.enabledChannels);
	}
}

QCoro::Task<CommandResponse<void>> Stream::setEnabledChannelsAsync(const QList<int> &channels)
{
	// Frame copy: the caller's list may not outlive the first suspension.
	const QList<int> resolved = resolveEnabledChannels(channels);
	if(!m_open) {
		StreamConfig cfg = m_config;
		cfg.enabledChannels = resolved;
		commitConfig(cfg);
		co_return CommandResponse<void>(QUuid());
	}

	const size_t samplesCount = m_config.samplesCount;
	auto closed = co_await closeAsync();
	if(!closed) {
		co_return closed;
	}
	if(resolved.isEmpty()) {
		StreamConfig cfg = m_config;
		cfg.enabledChannels = {};
		commitConfig(cfg);
		co_return closed;
	}
	co_return co_await openAsync({resolved, samplesCount});
}
