#include <QHBoxLayout>
#include <QSpacerItem>

#include <scopy/gui/channel_manager.hpp>

using namespace scopy::gui;

ChannelManager::ChannelManager(ChannelsPositionEnum position, QWidget* parent)
	: QWidget(parent)
	, m_channelsList(new QWidget)
	, m_position(position)
	, m_hasAddBtn(false)
	, m_addChannelBtn(new CustomPushButton)
{
	if (m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsList->setLayout(new QVBoxLayout);
	} else {
		m_channelsList->setLayout(new QHBoxLayout);
	}

	m_channelsList->layout()->setSpacing(0);
	m_channelsList->layout()->setMargin(0);
}

ChannelManager::~ChannelManager()
{
	delete m_channelsList;
	delete m_addChannelBtn;
}

void ChannelManager::build(QWidget* parent)
{
	m_parent = parent;
	m_parent->layout()->addWidget(m_channelsList);
}

ChannelWidget* ChannelManager::buildNewChannel(int chId, bool deletable, bool simplefied, QColor color,
					       const QString& fullName, const QString& shortName)
{
	ChannelWidget* ch = new ChannelWidget(chId, deletable, simplefied, color);

	m_channelsList->layout()->addWidget(ch);
	ch->setFullName(fullName + QString(" %1").arg(chId + 1));
	ch->setShortName(shortName + QString(" %1").arg(chId + 1));
	ch->nameButton()->setText(ch->shortName());

	return ch;
}

void ChannelManager::removeChannel(ChannelWidget* ch) { m_channelsList->layout()->removeWidget(ch); }

CustomPushButton* ChannelManager::getAddChannelBtn()
{
	if (m_hasAddBtn) {
		return m_addChannelBtn;
	} else {
		return nullptr;
	}
}

QWidget* ChannelManager::getChannelsList() { return m_channelsList; }

QList<ChannelWidget*> ChannelManager::getEnabledChannels()
{
	QList<ChannelWidget*> lst;

	for (auto widget : m_channelsList->children()) {
		ChannelWidget* channel = qobject_cast<ChannelWidget*>(widget);
		if (channel->enableButton()->isChecked()) {
			lst.append(channel);
		}
	}

	return lst;
}

void ChannelManager::insertAddBtn(QWidget* menu)
{
	m_hasAddBtn = true;

	// TO DO: center + btn when position is vertical
	m_addChannelBtn->setCheckable(true);
	m_addChannelBtn->setFlat(true);
	m_addChannelBtn->setIcon(QIcon(":/icons/common/add.svg"));
	m_addChannelBtn->setIconSize(QSize(16, 16));
	m_addChannelBtn->setMaximumSize(25, 25);

	m_parent->layout()->addWidget(m_addChannelBtn);

	Q_EMIT(configureAddBtn(menu));
}
