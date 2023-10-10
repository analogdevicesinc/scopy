#include "channel_manager.hpp"

#include "dynamicWidget.h"

#include <QHBoxLayout>

using namespace scopy;
using namespace scopy::gui;

ChannelManager::ChannelManager(ChannelsPositionEnum position, QWidget *parent)
	: QWidget(parent)
	, m_scrollArea(new QScrollArea(parent))
	, m_channelsWidget(new QWidget(m_scrollArea))
	, m_hasAddBtn(false)
	, m_switchBtn(new QPushButton(m_scrollArea))
	, m_addChannelBtn(new CustomPushButton()) // check if parent removed
	, m_position(position)
	, m_channelsList(QList<ChannelWidget *>())
	, m_channelIdVisible(true)
	, m_maxChannelWidth(-Q_INFINITY)
	, m_selectedChannel(-1)
{
	m_channelsWidget->setLayout(new QVBoxLayout(m_channelsWidget));

	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	header = new QWidget();
	auto headerLayout = new QHBoxLayout(header);
	header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	headerLayout->setMargin(10);
	headerLayout->setSpacing(10);

	toolStatus = new QLabel("");
	channelManagerToggled = false;

	QStringList icons = QStringList() << ":/gui/icons/scopy-default/icons/menu.svg";

	QIcon my_icon;
	my_icon.addFile(icons[0], QSize(), QIcon::Normal);

	toggleChannels = new QPushButton(this);
	toggleChannels->setStyleSheet("font-size: 12px; color: rgba(255, 255, 255, 70);");
	toggleChannels->setIcon(my_icon);
	toggleChannels->setIconSize(QSize(24, 24));
	toggleChannels->setFixedWidth(20);
	toggleChannels->setCheckable(true);
	toggleChannels->setChecked(false);
	connect(toggleChannels, &QPushButton::toggled, this, &ChannelManager::toggleChannelManager);

	headerLayout->addWidget(toggleChannels);
	headerLayout->addWidget(toolStatus);
	m_channelsWidget->layout()->addWidget(header);

	m_channelsWidget->layout()->setSpacing(0);
	m_channelsWidget->layout()->setMargin(0);
	m_channelsWidget->layout()->setContentsMargins(QMargins(0, 0, 0, 0));
	m_channelsWidget->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
	m_channelsWidget->setStyleSheet("border: 0px;");

	m_scrollArea->setWidget(m_channelsWidget);

	m_switchBtn->setVisible(false);
}

ChannelManager::~ChannelManager()
{
	delete m_channelsWidget;
	if(m_addChannelBtn) {
		delete m_addChannelBtn;
	}
	delete m_switchBtn;
	delete m_parent;
}

void ChannelManager::build(QWidget *parent)
{
	m_parent = parent;

	// Experimental - change orientation at runtime
	setDynamicProperty(m_switchBtn, "blue_button", true);
	m_switchBtn->setCheckable(true);
	m_switchBtn->setFlat(true);
	m_switchBtn->setText("Switch");
	connect(m_switchBtn, &QPushButton::toggled, [=]() {
		if(m_position == ChannelsPositionEnum::VERTICAL) {
			m_position = ChannelsPositionEnum::HORIZONTAL;

			//			if (channelManagerToggled) {
			//				toggleChannels->click();
			//			}
		} else {
			m_position = ChannelsPositionEnum::VERTICAL;
		}
		Q_EMIT positionChanged(m_position);
	});

	m_parent->layout()->addWidget(m_switchBtn);
	m_parent->layout()->addWidget(m_scrollArea);
}

void ChannelManager::updatePosition(ChannelsPositionEnum position)
{
	m_position = position == ChannelsPositionEnum::VERTICAL ? ChannelsPositionEnum::HORIZONTAL
								: ChannelsPositionEnum::VERTICAL;
	Q_EMIT m_switchBtn->toggled(true);
}

int ChannelManager::getChannelID(ChannelWidget *ch) { return m_channelsList.indexOf(ch); }

ChannelWidget *ChannelManager::buildNewChannel(int chId, bool deletable, bool simplefied, QColor color,
					       const QString &fullName, const QString &shortName)
{
	ChannelWidget *ch = new ChannelWidget(chId, deletable, simplefied, color);
	m_channelsWidget->layout()->setMargin(0);
	m_channelsWidget->layout()->addWidget(ch);
	if(m_channelIdVisible) {
		ch->setFullName(fullName + QString(" %1").arg(chId + 1));
		ch->setShortName(shortName + QString(" %1").arg(chId + 1));
	} else {
		ch->setFullName(fullName);
		ch->setShortName(shortName);
	}
	ch->nameButton()->setText(ch->shortName());

	m_channelsList.append(ch);

	connect(ch, &ChannelWidget::selected, this, &ChannelManager::onChannelSelected);
	connect(ch, &ChannelWidget::enabled, this, &ChannelManager::onChannelEnabled);
	connect(ch, &ChannelWidget::deleteClicked, this, &ChannelManager::onChannelDeleted);

	if(m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setMinimumHeight(m_channelsList.size() * m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.size() * m_channelsList.first()->height());
		if(m_maxChannelWidth < ch->sizeHint().width()) {
			m_minChannelWidth = ch->minimumWidth();
			m_maxChannelWidth = ch->sizeHint().width();
			m_channelsWidget->setMinimumWidth(m_minChannelWidth);
			m_channelsWidget->setMaximumWidth(m_maxChannelWidth);

			m_parent->setMinimumWidth(m_maxChannelWidth);
			m_parent->setMaximumWidth(m_maxChannelWidth);
		} else {
			// RESIZE CHANNELS
			for(auto c : qAsConst(m_channelsList)) {
				c->setMinimumWidth(m_maxChannelWidth);
			}
		}

	} else {
		if(m_maxChannelWidth < ch->sizeHint().width()) {
			m_minChannelWidth = ch->minimumWidth();
			m_maxChannelWidth = ch->sizeHint().width();
		}
		m_channelsWidget->setMinimumWidth(m_channelsList.size() * m_channelsList.first()->width());
		m_channelsWidget->setMaximumWidth(m_channelsList.size() * m_channelsList.first()->width());

		m_channelsWidget->setMinimumHeight(m_channelsList.first()->height());
		m_channelsWidget->setMaximumHeight(m_channelsList.first()->height());
	}

	ch->enableButton()->setChecked(true);
	updatePosition(m_position);
	// fake signal
	toggleChannels->setChecked(true);
	Q_EMIT toggleChannels->toggled(true);
	return ch;
}

void ChannelManager::removeChannel(ChannelWidget *ch)
{
	m_channelsList.removeOne(ch);
	m_channelsWidget->layout()->removeWidget(ch);
	delete ch;
}

CustomPushButton *ChannelManager::getAddChannelBtn()
{
	if(m_hasAddBtn) {
		return m_addChannelBtn;
	} else {
		return nullptr;
	}
}

QList<ChannelWidget *> ChannelManager::getChannelsList() { return m_channelsList; }

int ChannelManager::getChannelsCount() { return m_channelsList.size(); }

void ChannelManager::changeParent(QWidget *newParent)
{
	delete m_channelsWidget->layout();

	int ch_width = 0, ch_height = 0;
	if(m_channelsList.empty()) {
		ch_width = header->width();
		ch_height = header->height();
	} else {
		for(auto ch : m_channelsList) {
			ch_width = std::max(ch_width, ch->width());
			ch_height = std::max(ch_height, ch->height());
		}
	}

	if(m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setLayout(new QVBoxLayout(m_channelsWidget));
		m_channelsWidget->layout()->addWidget(header);

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		m_channelsWidget->setLayout(new QHBoxLayout(m_channelsWidget));
		m_channelsWidget->layout()->addWidget(header);

		m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}

	m_channelsWidget->layout()->setSpacing(0);
	m_channelsWidget->layout()->setMargin(0);
	m_channelsWidget->layout()->setContentsMargins(QMargins(0, 0, 0, 0));
	m_channelsWidget->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);
	m_channelsWidget->setStyleSheet("border: 0px;");

	for(ChannelWidget *channel : qAsConst(m_channelsList)) {
		m_channelsWidget->layout()->addWidget(channel);
	}

	if(m_position == ChannelsPositionEnum::VERTICAL) {
		m_channelsWidget->setMinimumHeight(m_channelsList.size() * ch_height);
		m_channelsWidget->setMaximumHeight(m_channelsList.size() * ch_height);

		m_channelsWidget->setMinimumWidth(ch_width + 25);
		m_channelsWidget->setMaximumWidth(ch_width + 25);
	} else {
		m_channelsWidget->setMinimumWidth(m_channelsList.size() * ch_width + 25);
		m_channelsWidget->setMaximumWidth(m_channelsList.size() * ch_width + 25);

		m_channelsWidget->setMinimumHeight(ch_height);
		m_channelsWidget->setMaximumHeight(ch_height);
	}

	m_parent = newParent;
	m_parent->layout()->addWidget(m_switchBtn);
	m_parent->layout()->addWidget(m_scrollArea);
	if(m_hasAddBtn) {
		m_parent->layout()->addWidget(m_addChannelBtn);
	}

	m_parent->layout()->setAlignment(m_addChannelBtn, Qt::AlignHCenter);
	m_parent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_channelsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	if(m_position == ChannelsPositionEnum::VERTICAL) {
		m_parent->setMaximumHeight(INT_MAX);

		m_parent->setMaximumWidth(m_maxChannelWidth);
		//		m_parent->setMinimumWidth(m_maxChannelWidth);
	} else {
		m_parent->setMinimumHeight(ch_height + 8);
		m_parent->setMaximumHeight(ch_height + 8);

		m_parent->setMaximumWidth(INT_MAX);
		//		m_parent->setMinimumWidth(ch_width);
	}
}

QList<ChannelWidget *> ChannelManager::getEnabledChannels()
{
	QList<ChannelWidget *> lst;

	for(auto widget : m_channelsWidget->children()) {
		ChannelWidget *channel = qobject_cast<ChannelWidget *>(widget);
		if(channel->enableButton()->isChecked()) {
			lst.append(channel);
		}
	}

	return lst;
}

void ChannelManager::enableSwitchButton(bool en) { m_switchBtn->setVisible(en); }

void ChannelManager::insertAddBtn(QWidget *menu, bool dockable)
{

	m_hasAddBtn = true;
	m_addChannelBtn = new CustomPushButton(m_scrollArea);

	m_addChannelBtn->setCheckable(true);
	m_addChannelBtn->setFlat(true);
	m_addChannelBtn->setIcon(QIcon(":/gui/icons/add.svg"));
	m_addChannelBtn->setIconSize(QSize(25, 25));
	m_addChannelBtn->setMaximumSize(25, 25);

	m_parent->layout()->addWidget(m_addChannelBtn);
	m_parent->layout()->setAlignment(m_addChannelBtn, Qt::AlignHCenter);

	Q_EMIT(configureAddBtn(menu, dockable));
}

void ChannelManager::setChannelAlignment(ChannelWidget *ch, Qt::Alignment alignment)
{
	m_channelsWidget->layout()->setAlignment(ch, alignment);
}

void ChannelManager::setChannelIdVisible(bool visible) { m_channelIdVisible = visible; }

void ChannelManager::toggleChannelManager(bool toggled)
{
	Q_EMIT channelManagerToggle(toggled);

	for(auto ch : qAsConst(m_channelsList)) {
		ch->toggleChannel(!toggled);
		if(ch->isMainChannel() || ch->isPhysicalChannel()) {
			ch->setMenuButtonVisibility(toggled);
		}
	}
	toolStatus->setVisible(toggled);

	auto currentWidth = 0;
	if(!toggled) {
		currentWidth = m_minChannelWidth;
	} else {
		currentWidth = m_maxChannelWidth;
	}

	m_channelsWidget->setMinimumWidth(currentWidth);
	m_channelsWidget->setMaximumWidth(currentWidth);

	// RESIZE CHANNELS
	for(auto c : qAsConst(m_channelsList)) {
		c->setMinimumWidth(currentWidth);
	}
}

int ChannelManager::getSelectedChannel() { return m_selectedChannel; }

void ChannelManager::onChannelSelected(bool toggled)
{
	ChannelWidget *ch = dynamic_cast<ChannelWidget *>(sender());
	if(ch != nullptr) {
		int id = m_channelsList.indexOf(ch);
		m_selectedChannel = id;
		Q_EMIT selectedChannel(id, toggled);
	}
}

void ChannelManager::onChannelEnabled(bool enabled)
{
	ChannelWidget *ch = dynamic_cast<ChannelWidget *>(sender());
	if(ch != nullptr) {
		int id = m_channelsList.indexOf(ch);

		Q_EMIT enabledChannel(id, enabled);
	}
}

void ChannelManager::onChannelDeleted()
{
	ChannelWidget *ch = dynamic_cast<ChannelWidget *>(sender());
	if(ch != nullptr) {

		Q_EMIT deletedChannel(ch->shortName());
	}
}

const ChannelWidget *ChannelManager::getChannelAt(int id) { return m_channelsList.at(id); }

const QString &ChannelManager::getToolStatus() const
{
	static const auto text = toolStatus->text();
	return text;
}

void ChannelManager::setToolStatus(const QString &newToolStatus) { toolStatus->setText(newToolStatus); }

#include "moc_channel_manager.cpp"
