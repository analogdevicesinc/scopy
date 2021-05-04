#include "ui_tool_view.h"

#include <QDebug>

#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/tool_view.hpp>

using namespace scopy::gui;

ToolView::ToolView(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ToolView)
{
	m_ui->setupUi(this);

	m_ui->widgetRunSingleBtns->enableRunButton(false);
	m_ui->widgetRunSingleBtns->enableSingleButton(false);
	m_ui->widgetSettingsPairBtns->setVisible(false);
	m_ui->btnHelp->setVisible(false);
	m_ui->btnPrint->setVisible(false);

	m_ui->widgetInstrumentNotes->setVisible(false);
	m_ui->widgetVerticalChannels->setVisible(false);
	m_ui->widgetFooter->setVisible(false);

	m_ui->widgetMenuAnim->setMaximumWidth(0);

	connect(m_ui->widgetMenuAnim, &MenuAnim::finished, this, &ToolView::rightMenuFinished);
}

ToolView::~ToolView() { delete m_ui; }

void ToolView::configurePairSettings()
{
	CustomPushButton* generalSettingsBtn = m_ui->widgetSettingsPairBtns->getGeneralSettingsBtn();
	QPushButton* settingsBtn = m_ui->widgetSettingsPairBtns->getSettingsBtn();

	// General settings
	int id = m_ui->stackedWidget->indexOf(m_ui->widgetGeneralMenu);

	generalSettingsBtn->setProperty("id", QVariant(-id));
	m_group.addButton(generalSettingsBtn);

	connect(generalSettingsBtn, &CustomPushButton::toggled, this, [=](bool toggled) {
		triggerRightMenuToggle(toggled);
		if (toggled) {
			settingsBtn->setChecked(!toggled);
		}
	});

	//	generalSettingsBtn->setChecked(true);

	// Settings/Last opened menu
	connect(settingsBtn, &QPushButton::clicked, this, [=](bool checked) {
		if (!m_menuOrder.isEmpty()) {
			CustomPushButton* btn = nullptr;

			if (checked) {
				btn = m_menuOrder.back();
				m_menuOrder.pop_back();
			} else {
				btn = static_cast<CustomPushButton*>(m_group.checkedButton());
			}

			btn->setChecked(checked);
		} else {
			getSettingsBtn()->setChecked(false);
		}
	});
}

void ToolView::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (m_menuButtonActions.size()) {
		auto pair = m_menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void ToolView::triggerRightMenuToggle(bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	CustomPushButton* btn = static_cast<CustomPushButton*>(QObject::sender());
	if (m_ui->widgetMenuAnim->animInProgress()) {
		m_menuButtonActions.enqueue(QPair<CustomPushButton*, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void ToolView::toggleRightMenu(CustomPushButton* btn, bool checked)
{
	int id = btn->property("id").toInt();
	if (id != -m_ui->stackedWidget->indexOf(m_ui->widgetGeneralMenu)) {
		if (!m_menuOrder.contains(btn)) {
			m_menuOrder.push_back(btn);
		} else {
			m_menuOrder.removeOne(btn);
			m_menuOrder.push_back(btn);
		}
	}

	if (checked) {
		settingsPanelUpdate(id);
	}

	m_ui->widgetMenuAnim->toggleMenu(checked);
}

void ToolView::settingsPanelUpdate(int id)
{
	if (id >= 0) {
		m_ui->stackedWidget->setCurrentIndex(0);
	} else {
		m_ui->stackedWidget->setCurrentIndex(-id);
	}

	for (int i = 0; i < m_ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;

		if (i == m_ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
		}
		QWidget* widget = m_ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
	}
	m_ui->stackedWidget->adjustSize();
}

void ToolView::buildChannelsContainer(ChannelManager* cm, ChannelsPositionEnum position)
{
	connect(cm, &ChannelManager::configureAddBtn, this, &ToolView::configureAddMathBtn);

	if (position == ChannelsPositionEnum::HORIZONTAL) {
		m_ui->widgetFooter->setVisible(true);

		cm->build(m_ui->widgetHorizontalChannelsContainer);
	} else {
		m_ui->widgetVerticalChannels->setVisible(true);
		cm->build(m_ui->widgetVerticalChannelsContainer);
	}
}

void ToolView::configureAddMathBtn(QWidget* menu)
{
	ChannelManager* cm = static_cast<ChannelManager*>(QObject::sender());

	int id = m_ui->stackedWidget->addWidget(menu);

	CustomPushButton* addBtn = cm->getAddChannelBtn();

	m_group.addButton(addBtn);
	addBtn->setProperty("id", QVariant(-id));

	connect(addBtn, &CustomPushButton::toggled, this, &ToolView::triggerRightMenuToggle);
	connect(addBtn, &CustomPushButton::toggled, m_ui->widgetSettingsPairBtns->getSettingsBtn(),
		&QPushButton::setChecked);
}

ChannelWidget* ToolView::buildNewChannel(ChannelManager* channelManager, QWidget* menu, int chId, bool deletable,
					 bool simplefied, QColor color, const QString& fullName,
					 const QString& shortName)
{
	ChannelWidget* ch = channelManager->buildNewChannel(chId, deletable, simplefied, color, fullName, shortName);
	int id = m_ui->stackedWidget->addWidget(menu);

	m_group.addButton(ch->menuButton());
	m_channelsGroup.addButton(ch->nameButton());
	ch->menuButton()->setProperty("id", QVariant(-id));

	connect(ch->menuButton(), &CustomPushButton::toggled, this, &ToolView::triggerRightMenuToggle);

	connect(ch->menuButton(), &CustomPushButton::toggled, m_ui->widgetSettingsPairBtns->getSettingsBtn(),
		&QPushButton::setChecked);

	connect(ch->enableButton(), &QAbstractButton::toggled, [=](bool toggled) {
		if (!toggled) {
			// we also remove the button from the history
			// so that the last menu opened button on top
			// won't open the menu when it is disabled
			m_menuOrder.removeOne(qobject_cast<CustomPushButton*>(ch->menuButton()));
		}
	});

	if (deletable) {
		connect(ch, &ChannelWidget::deleteClicked, this, [=]() {
			if (ch->menuButton()->isChecked()) {
				m_menuButtonActions.removeAll(QPair<CustomPushButton*, bool>(
					qobject_cast<CustomPushButton*>(ch->menuButton()), true));
				toggleRightMenu(qobject_cast<CustomPushButton*>(ch->menuButton()), false);
			}
			m_menuOrder.removeOne(qobject_cast<CustomPushButton*>(ch->menuButton()));

			channelManager->removeChannel(ch);
		});
	}

	return ch;
}

void ToolView::buildNewInstrumentMenu(QWidget* menu, const QString& name, bool checkBoxVisible, bool checkBoxChecked)
{
	m_ui->widgetFooter->setVisible(true);

	int id = m_ui->stackedWidget->addWidget(menu);

	CustomMenuButton* btn = new CustomMenuButton(name, checkBoxVisible, checkBoxChecked);
	m_ui->hLayoutMenuBtnsContainer->addWidget(btn);
	m_group.addButton(btn->getBtn());
	btn->getBtn()->setProperty("id", QVariant(-id));

	connect(btn->getBtn(), &CustomPushButton::toggled, this, &ToolView::triggerRightMenuToggle);
	connect(btn->getBtn(), &CustomPushButton::toggled, m_ui->widgetSettingsPairBtns->getSettingsBtn(),
		&QPushButton::setChecked);
	connect(btn->getCheckBox(), &QCheckBox::toggled, [=](bool toggled) {
		if (!toggled) {
			// we also remove the button from the history
			// so that the last menu opened button on top
			// won't open the menu when it is disabled
			m_menuOrder.removeOne(btn->getBtn());
		}
	});

	m_menuOrder.push_back(btn->getBtn());
}

void ToolView::setGeneralSettingsMenu(QWidget* menu)
{
	m_ui->widgetGeneralMenu->setLayout(new QHBoxLayout);
	m_ui->widgetGeneralMenu->layout()->addWidget(menu);
}

void ToolView::setFixedMenu(QWidget* menu)
{
	int id = m_ui->stackedWidget->addWidget(menu);
	settingsPanelUpdate(id);
	m_ui->widgetMenuAnim->toggleMenu(true);
}

QWidget* ToolView::getTopExtraWidget() { return m_ui->widgetTopExtra; }

void ToolView::setVisibleTopExtraWidget(bool visible) { m_ui->widgetTopExtra->setVisible(visible); }

void ToolView::addTopExtraWidget(QWidget* widget) { m_ui->widgetTopExtra->layout()->addWidget(widget); }

QWidget* ToolView::getBottomExtraWidget() { return m_ui->widgetBottomExtra; }

void ToolView::setVisibleBottomExtraWidget(bool visible) { m_ui->widgetBottomExtra->setVisible(visible); }

void ToolView::addBottomExtraWidget(QWidget* widget) { m_ui->widgetBottomExtra->layout()->addWidget(widget); }

QWidget* ToolView::getCentralWidget() { return m_ui->widgetCentral; }

void ToolView::setInstrumentNotesVisible(bool visible) { m_ui->widgetInstrumentNotes->setVisible(visible); }

LinkedButton* ToolView::getHelpBtn() { return m_ui->btnHelp; }

void ToolView::setHelpBtnVisible(bool visible) { m_ui->btnHelp->setVisible(visible); }

void ToolView::setUrlHelpBtn(const QString& url) { m_ui->btnHelp->setUrl(url); }

QPushButton* ToolView::getRunBtn() { return m_ui->widgetRunSingleBtns->getRunButton(); }

void ToolView::setRunBtnVisible(bool visible) { m_ui->widgetRunSingleBtns->enableRunButton(visible); }

QPushButton* ToolView::getSingleBtn() { return m_ui->widgetRunSingleBtns->getSingleButton(); }

void ToolView::setSingleBtnVisible(bool visible) { m_ui->widgetRunSingleBtns->enableSingleButton(visible); }

QPushButton* ToolView::getPrintBtn() { return m_ui->btnPrint; }

void ToolView::setPrintBtnVisible(bool visible) { m_ui->btnPrint->setVisible(visible); }

void ToolView::setPairSettingsVisible(bool visible) { m_ui->widgetSettingsPairBtns->setVisible(visible); }

CustomPushButton* ToolView::getGeneralSettingsBtn() { return m_ui->widgetSettingsPairBtns->getGeneralSettingsBtn(); }

QPushButton* ToolView::getSettingsBtn() { return m_ui->widgetSettingsPairBtns->getSettingsBtn(); }
