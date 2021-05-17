#include "dynamic_widget.hpp"

#include <QHBoxLayout>

#include <scopy/gui/channel_manager.hpp>

using namespace scopy::gui;

ChannelManager::ChannelManager(ChannelsPositionEnum position, QWidget* parent)
	: QWidget(parent)
	, m_scrollArea(new QScrollArea(parent))
	, m_channelsWidget(new QWidget(m_scrollArea))
	, m_switchBtn(new QPushButton(m_scrollArea))
	, m_hasAddBtn(false)
	, m_addChannelBtn(new CustomPushButton(m_scrollArea))
	, m_position(position)
{
	if (m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setLayout(new QVBoxLayout(m_channelsWidget));

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		m_channelsWidget->setLayout(new QHBoxLayout(m_channelsWidget));

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}

	m_channelsWidget->layout()->setSpacing(0);
	m_channelsWidget->layout()->setMargin(0);
	m_channelsWidget->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

	m_scrollArea->setWidget(m_channelsWidget);

	m_scrollArea->setStyleSheet("background-color:orange");
	m_channelsWidget->setStyleSheet("background-color:red");
}

ChannelManager::~ChannelManager()
{
	delete m_channelsWidget;
	delete m_addChannelBtn;
	delete m_switchBtn;
	delete m_parent;
}

void ChannelManager::build(QWidget* parent)
{
	m_parent = parent;

	// Experimental - change orientation at runtime
	DynamicWidget::setDynamicProperty(m_switchBtn, "blue_button", true);
	m_switchBtn->setCheckable(true);
	m_switchBtn->setFlat(true);
	m_switchBtn->setText("Switch");
	connect(m_switchBtn, &QPushButton::toggled, [=]() {
		if (m_position == ChannelsPositionEnum::VERTICAL) {
			m_position = ChannelsPositionEnum::HORIZONTAL;
		} else {
			m_position = ChannelsPositionEnum::VERTICAL;
		}
		Q_EMIT positionChanged(m_position);
	});

	m_parent->layout()->addWidget(m_switchBtn);
	m_parent->layout()->addWidget(m_scrollArea);
}

ChannelWidget* ChannelManager::buildNewChannel(int chId, bool deletable, bool simplefied, QColor color,
					       const QString& fullName, const QString& shortName)
{
	ChannelWidget* ch = new ChannelWidget(chId, deletable, simplefied, color);

	m_channelsWidget->layout()->addWidget(ch);
	ch->setFullName(fullName + QString(" %1").arg(chId + 1));
	ch->setShortName(shortName + QString(" %1").arg(chId + 1));
	ch->nameButton()->setText(ch->shortName());

	m_channelsList.append(ch);

	if (m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setMinimumHeight(m_channelsList.size() * m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.size() * m_channelsList.first()->height());

		m_channelsWidget->setMinimumWidth(m_channelsList.last()->width());
		m_channelsWidget->setMaximumWidth(m_channelsList.last()->width());
	} else {
		m_channelsWidget->setMinimumWidth(m_channelsList.size() * m_channelsList.first()->width());
		m_channelsWidget->setMaximumWidth(m_channelsList.size() * m_channelsList.first()->width());

		m_channelsWidget->setMinimumHeight(m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.first()->height());
	}

	return ch;
}

void ChannelManager::removeChannel(ChannelWidget* ch)
{
	m_channelsList.removeOne(ch);
	m_channelsWidget->layout()->removeWidget(ch);
	delete ch;
}

CustomPushButton* ChannelManager::getAddChannelBtn()
{
	if (m_hasAddBtn) {
		return m_addChannelBtn;
	} else {
		return nullptr;
	}
}

QList<ChannelWidget*> ChannelManager::getChannelsList() { return m_channelsList; }

void ChannelManager::changeParent(QWidget* newParent)
{
	delete m_channelsWidget->layout();

	if (m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setLayout(new QVBoxLayout(m_channelsWidget));

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		m_channelsWidget->setLayout(new QHBoxLayout(m_channelsWidget));

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}

	m_channelsWidget->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
	m_channelsWidget->layout()->setSpacing(0);
	m_channelsWidget->layout()->setMargin(0);

	for (ChannelWidget* channel : m_channelsList) {
		m_channelsWidget->layout()->addWidget(channel);
	}

	if (m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setMinimumHeight(m_channelsList.size() * m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.size() * m_channelsList.first()->height());

		m_channelsWidget->setMinimumWidth(m_channelsList.last()->width() + 25);
		m_channelsWidget->setMaximumWidth(m_channelsList.last()->width() + 25);
	} else {
		m_channelsWidget->setMinimumWidth(m_channelsList.size() * m_channelsList.first()->width());
		m_channelsWidget->setMaximumWidth(m_channelsList.size() * m_channelsList.first()->width());

		m_channelsWidget->setMinimumHeight(m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.first()->height());
	}

	m_parent = newParent;
	m_parent->layout()->addWidget(m_switchBtn);
	m_parent->layout()->addWidget(m_scrollArea);
	m_parent->layout()->addWidget(m_addChannelBtn);
}

QList<ChannelWidget*> ChannelManager::getEnabledChannels()
{
	QList<ChannelWidget*> lst;

	for (auto widget : m_channelsWidget->children()) {
		ChannelWidget* channel = qobject_cast<ChannelWidget*>(widget);
		if (channel->enableButton()->isChecked()) {
			lst.append(channel);
		}
	}

	return lst;
}

void ChannelManager::insertAddBtn(QWidget* menu, bool dockable)
{
	m_hasAddBtn = true;

	// TO DO: center + btn when position is vertical
	m_addChannelBtn->setCheckable(true);
	m_addChannelBtn->setFlat(true);
	m_addChannelBtn->setIcon(QIcon(":/icons/common/add.svg"));
	m_addChannelBtn->setIconSize(QSize(16, 16));
	m_addChannelBtn->setMaximumSize(25, 25);

	m_parent->layout()->addWidget(m_addChannelBtn);

	Q_EMIT(configureAddBtn(menu, dockable));
}
