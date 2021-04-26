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
	m_ui->btnAddMath->setVisible(false);

	m_ui->widgetInstrumentNotes->setVisible(false);

	m_ui->widgetMenuAnim->setMaximumWidth(0);

	connect(m_ui->widgetMenuAnim, &MenuAnim::finished, this, &ToolView::rightMenuFinished);
}

ToolView::~ToolView()
{
	delete m_generalSettingsMenu;
	delete m_ui;
}

void ToolView::setBtns(QMap<MenusEnum, CustomMenuButton*> btns)
{
	m_btns = btns;

	for (MenusEnum key : btns.keys()) {
		QWidget* menu = new QWidget;
		m_menus.insert(MenusEnum(key), menu);

		int id = m_ui->stackedWidget->addWidget(menu);

		CustomMenuButton* btn = qobject_cast<CustomMenuButton*>(btns.value(key));
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
}

void ToolView::setUpperMenus()
{
	CustomPushButton* generalSettingsBtn = m_ui->widgetSettingsPairBtns->getGeneralSettingsBtn();
	QPushButton* settingsBtn = m_ui->widgetSettingsPairBtns->getSettingsBtn();

	// General settings
	m_generalSettingsMenu = new QWidget;
	int id = m_ui->stackedWidget->addWidget(m_generalSettingsMenu);

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
		CustomPushButton* btn = nullptr;
		if (checked && !m_menuOrder.isEmpty()) {
			btn = m_menuOrder.back();
			m_menuOrder.pop_back();
		} else {
			btn = static_cast<CustomPushButton*>(m_group.checkedButton());
		}

		btn->setChecked(checked);
	});
}

QWidget* ToolView::getCentralWidget() { return m_ui->widgetCentral; }

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
	if (id != -m_ui->stackedWidget->indexOf(m_generalSettingsMenu)) {
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

QWidget* ToolView::getMenu(MenusEnum menu) { return m_menus[menu]; }

QWidget *ToolView::getGeneralSettingsMenu()
{
	return m_generalSettingsMenu;
}

QWidget* ToolView::getButtonMenu(MenusEnum btn) { return m_btns[btn]; }

void ToolView::setPrintBtnVisible(bool visible) { m_ui->btnPrint->setVisible(visible); }

void ToolView::setRunBtnVisible(bool visible) { m_ui->widgetRunSingleBtns->enableRunButton(visible); }

void ToolView::setSingleBtnVisible(bool visible) { m_ui->widgetRunSingleBtns->enableSingleButton(visible); }

void ToolView::setPairSettingsVisible(bool visible) { m_ui->widgetSettingsPairBtns->setVisible(visible); }

QWidget* ToolView::getExtraWidget() { return m_ui->widgetExtraWidget; }

void ToolView::setExtraWidget(QWidget* widget) { m_ui->hLayoutExtraWidgets->addWidget(widget); }

void ToolView::setHelpBtnVisible(bool visible) { m_ui->btnHelp->setVisible(visible); }

void ToolView::setInstrumentNotesVisible(bool visible) { m_ui->widgetInstrumentNotes->setVisible(visible); }

CustomPushButton* ToolView::getAddMathBtn() { return m_ui->btnAddMath; }

void ToolView::setAddMathBtnVisible(bool visible) { m_ui->btnAddMath->setVisible(visible); }

void ToolView::configureAddMathBtn()
{
	setAddMathBtnVisible(true);

	QWidget* menu = new QWidget;
	m_menus.insert(MenusEnum::ADD_CHANNEL, menu);
	int id = m_ui->stackedWidget->addWidget(menu);

	m_group.addButton(getAddMathBtn());
	getAddMathBtn()->setProperty("id", QVariant(-id));

	connect(getAddMathBtn(), &CustomPushButton::toggled, this, &ToolView::triggerRightMenuToggle);

	connect(getAddMathBtn(), &CustomPushButton::toggled, m_ui->widgetSettingsPairBtns->getSettingsBtn(),
		&QPushButton::setChecked);
}

QWidget* ToolView::getChannels() { return m_ui->widgetChannelsList; }

void ToolView::buildDefaultChannels()
{
	QWidget* menu = new QWidget;

	// TODO: remove layout, find a way to only set parent
	menu->setLayout(new QVBoxLayout);
	menu->layout()->setMargin(0);

	m_menus.insert(MenusEnum::CHANNELS_SETTINGS, menu);
	m_ui->stackedWidget->addWidget(menu);

	buildNewChannel(0, false, false, QColor("#FF7200"), "Channel", "CH");
	buildNewChannel(1, false, false, QColor("#9013FE"), "Channel", "CH");
}

ChannelWidget* ToolView::buildNewChannel(int chId, bool deletable, bool simplefied, QColor color, const QString& fullName,
				   const QString& shortName)
{
	ChannelWidget* ch = new ChannelWidget(chId, deletable, simplefied, color, m_ui->widgetChannelsList);
	auto id = m_ui->stackedWidget->indexOf(getMenu(MenusEnum::CHANNELS_SETTINGS));

	m_ui->hLayoutChannelList->addWidget(ch);
	ch->setFullName(fullName + QString(" %1").arg(chId + 1));
	ch->setShortName(shortName + QString(" %1").arg(chId + 1));
	ch->nameButton()->setText(ch->shortName());

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

			m_ui->hLayoutChannelList->removeWidget(ch);
		});
	}

	return ch;
}

QStackedWidget* ToolView::getStackedWidget() { return m_ui->stackedWidget; }

void ToolView::setUrlHelpBtn(const QString& url) { m_ui->btnHelp->setUrl(url); }

QPushButton* ToolView::getRunBtn() { return m_ui->widgetRunSingleBtns->getRunButton(); }

QPushButton* ToolView::getSingleBtn() { return m_ui->widgetRunSingleBtns->getSingleButton(); }

LinkedButton* ToolView::getHelpBtn() { return m_ui->btnHelp; }

QPushButton* ToolView::getPrintBtn() { return m_ui->btnPrint; }

CustomPushButton* ToolView::getGeneralSettingsBtn() { return m_ui->widgetSettingsPairBtns->getGeneralSettingsBtn(); }

QPushButton* ToolView::getSettingsBtn() { return m_ui->widgetSettingsPairBtns->getSettingsBtn(); }
