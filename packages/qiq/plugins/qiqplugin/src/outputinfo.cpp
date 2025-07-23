#include "outputinfo.h"

#include <mapkeys.h>

using namespace scopy::qiqplugin;

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
