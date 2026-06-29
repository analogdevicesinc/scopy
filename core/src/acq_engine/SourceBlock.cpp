#include "SourceBlock.h"

#include "AcquisitionEngine.h"

#include <QWidget>

namespace scopy {
namespace acq {

SourceBlock::SourceBlock(const QString &id, QObject *parent)
	: QObject(parent)
	, m_id(id)
{}

void SourceBlock::report(AcquisitionError::Severity sev, const QString &msg) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	if(!engine)
		return;
	Q_EMIT engine->error(static_cast<int>(sev), m_id, msg);
}

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
	m_channels[channelId] = en;
}

void SourceBlock::disableAllChannels()
{
	for(auto &enabled : m_channels)
		enabled = false;
}

bool SourceBlock::isChannelEnabled(const QString &channelId) const
{
	auto it = m_channels.find(channelId);
	return it != m_channels.end() && it.value();
}

QList<QString> SourceBlock::enabledChannels() const
{
	QList<QString> result;
	for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
		if(it.value())
			result.append(it.key());
	}
	return result;
}

QWidget *SourceBlock::createSettingsWidget(QWidget *) { return nullptr; }

} // namespace acq
} // namespace scopy
