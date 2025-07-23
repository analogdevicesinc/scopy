/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "outputinfo.h"

#include <mapkeys.h>

using namespace scopy::extprocplugin;

OutputInfo::OutputInfo()
	: m_channelCount(0)
{}

void OutputInfo::fromVariantMap(const QVariantMap &params)
{
	m_channelCount = params.value(KeysOutputInfo::CHANNEL_COUNT, 0).toInt();
	m_channelNames = params.value(KeysOutputInfo::CHANNEL_NAMES).toStringList();
	m_channelFormat = params.value(KeysOutputInfo::CHANNEL_FORMAT).toStringList();
}

QVariantMap OutputInfo::toVariantMap() const
{
	QVariantMap map;
	map[KeysOutputInfo::CHANNEL_COUNT] = m_channelCount;
	map[KeysOutputInfo::CHANNEL_NAMES] = m_channelNames;
	map[KeysOutputInfo::CHANNEL_FORMAT] = m_channelFormat;
	return map;
}

int OutputInfo::channelCount() const { return m_channelCount; }
void OutputInfo::setChannelCount(int count) { m_channelCount = count; }

QStringList OutputInfo::channelNames() const { return m_channelNames; }
void OutputInfo::setChannelNames(const QStringList &names) { m_channelNames = names; }

QStringList OutputInfo::channelFormat() const { return m_channelFormat; }
void OutputInfo::setChannelFormat(const QStringList &format) { m_channelFormat = format; }

bool OutputInfo::isValid() { return true; }
