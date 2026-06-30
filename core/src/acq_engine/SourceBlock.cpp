#include "SourceBlock.h"

#include "AcquisitionEngine.h"

#include <QCheckBox>
#include <QVBoxLayout>
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
	const bool added = !m_channels.contains(channelId);
	m_channels[channelId] = en;
	if(added)
		Q_EMIT channelsChanged();
}

void SourceBlock::removeChannel(const QString &channelId)
{
	if(m_channels.remove(channelId) > 0)
		Q_EMIT channelsChanged();
}

void SourceBlock::setEnabled(bool en)
{
	const bool prev = m_sourceEnabled.exchange(en, std::memory_order_relaxed);
	if(prev != en)
		Q_EMIT enabledChanged(en);
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

QWidget *SourceBlock::createSettingsWidget(QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);

	auto *enableCb = new QCheckBox(QStringLiteral("Enabled"), w);
	enableCb->setChecked(isEnabled());
	connect(enableCb, &QCheckBox::toggled, this, [this](bool en) { setEnabled(en); });
	connect(this, &SourceBlock::enabledChanged, enableCb, &QCheckBox::setChecked);
	lay->addWidget(enableCb);

	auto *channelsContainer = new QWidget(w);
	auto *channelsLay       = new QVBoxLayout(channelsContainer);
	channelsLay->setContentsMargins(0, 0, 0, 0);
	channelsLay->setSpacing(2);
	lay->addWidget(channelsContainer);

	auto rebuildChannels = [this, channelsContainer, channelsLay]() {
		QLayoutItem *item;
		while((item = channelsLay->takeAt(0)) != nullptr) {
			if(QWidget *cw = item->widget())
				cw->deleteLater();
			delete item;
		}
		for(const QString &chId : channelIds()) {
			auto *cb = new QCheckBox(chId, channelsContainer);
			cb->setChecked(isChannelEnabled(chId));
			connect(cb, &QCheckBox::toggled, this,
				[this, chId](bool en) { enableChannel(chId, en); });
			channelsLay->addWidget(cb);
		}
	};

	connect(this, &SourceBlock::channelsChanged, w, rebuildChannels);
	rebuildChannels();

	return w;
}

} // namespace acq
} // namespace scopy
