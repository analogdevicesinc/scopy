#include "SourceBlock.h"

#include <QCheckBox>
#include <QMutexLocker>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
namespace acq {

SourceBlock::SourceBlock(const QString &id, QObject *parent)
	: Block(id, parent)
{}

void SourceBlock::onStart()
{
	m_stopRequested = false;
}

void SourceBlock::onStop()
{
	m_stopRequested = true;
}

void SourceBlock::setBufferSize(std::size_t size)
{
	m_bufferSize = size;
}

std::size_t SourceBlock::bufferSize() const
{
	return m_bufferSize;
}

void SourceBlock::enableChannel(const QString &channelId, bool en)
{
	bool added, flipped;
	{
		QMutexLocker lk(&m_channelMutex);
		added   = !m_channels.contains(channelId);
		flipped = m_channels.value(channelId, false) != en;
		m_channels[channelId] = en;
	}
	if(added)
		Q_EMIT channelsChanged();
	if(flipped)
		Q_EMIT channelEnabledChanged(channelId, en);
}

void SourceBlock::removeChannel(const QString &channelId)
{
	bool removed;
	{
		QMutexLocker lk(&m_channelMutex);
		removed = m_channels.remove(channelId) > 0;
	}
	if(removed)
		Q_EMIT channelsChanged();
}

void SourceBlock::disableAllChannels()
{
	QList<QString> flipped;
	{
		QMutexLocker lk(&m_channelMutex);
		for(auto it = m_channels.begin(); it != m_channels.end(); ++it) {
			if(!it.value())
				continue;
			it.value() = false;
			flipped.append(it.key());
		}
	}
	for(const QString &id : flipped)
		Q_EMIT channelEnabledChanged(id, false);
}

bool SourceBlock::isChannelEnabled(const QString &channelId) const
{
	QMutexLocker lk(&m_channelMutex);
	return m_channels.value(channelId, false);
}

QList<QString> SourceBlock::enabledChannels() const
{
	QMutexLocker lk(&m_channelMutex);
	QList<QString> result;
	for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it)
		if(it.value())
			result.append(it.key());
	return result;
}

QList<QString> SourceBlock::channelIds() const
{
	QMutexLocker lk(&m_channelMutex);
	return m_channels.keys();
}

QWidget *SourceBlock::createSettingsWidget(QWidget *parent)
{
	auto *channels = new QWidget;
	auto *lay      = new QVBoxLayout(channels);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(2);

	auto rebuild = [this, channels, lay]() {
		while(QLayoutItem *item = lay->takeAt(0)) {
			if(QWidget *cw = item->widget())
				cw->deleteLater();
			delete item;
		}
		for(const QString &chId : channelIds()) {
			auto *cb = new QCheckBox(chId, channels);
			cb->setChecked(isChannelEnabled(chId));
			connect(cb, &QCheckBox::toggled, this,
				[this, chId](bool en) { enableChannel(chId, en); });
			lay->addWidget(cb);
		}
	};
	connect(this, &SourceBlock::channelsChanged, channels, rebuild);
	rebuild();

	return withBaseSettings(channels, parent);
}

} // namespace acq
} // namespace scopy
