﻿#include "channel_widget.hpp"
//#include "menu_header.hpp"
#include "tool_view.hpp"
#include "utils.h"
#include <QTabWidget>
#include "ui_tool_view.h"
#include <QMainWindow>

using namespace scopy::gui;

ToolView::ToolView(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ToolView)
	, m_nextMenuIndex(0)
{
	m_ui->setupUi(this);

	m_centralMainWindow = new QMainWindow(m_ui->widgetCentral);
	m_centralMainWindow->setCentralWidget(0);
	m_centralMainWindow->setWindowFlags(Qt::Widget);
	m_ui->widgetCentral->layout()->addWidget(m_centralMainWindow);

	m_ui->widgetRunSingleBtns->enableRunButton(false);
	m_ui->widgetRunSingleBtns->enableSingleButton(false);
	m_ui->widgetSettingsPairBtns->setVisible(false);
	m_ui->btnHelp->setVisible(false);
	m_ui->btnPrint->setVisible(false);

	m_ui->widgetInstrumentNotes->setVisible(false);
	m_ui->widgetVerticalChannels->setVisible(false);
	m_ui->widgetFooter->setVisible(false);
	m_ui->widgetMenuBtns->setVisible(false);

	m_ui->widgetMenuHAnim->setMaximumWidth(0);

	getGeneralSettingsBtn()->setVisible(false);
	getSettingsBtn()->setVisible(false);

	connect(m_ui->widgetMenuHAnim, &MenuHAnim::finished, this, &ToolView::rightMenuFinished);
}

ToolView::~ToolView() { delete m_ui; }

void ToolView::configureLastOpenedMenu()
{
	QPushButton* settingsBtn = m_ui->widgetSettingsPairBtns->getSettingsBtn();

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
	if (m_ui->widgetMenuHAnim->animInProgress()) {
		m_menuButtonActions.enqueue(QPair<CustomPushButton*, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void ToolView::toggleRightMenu(CustomPushButton* btn, bool checked)
{
	int id = btn->property("id").toInt();

	if (id != -m_generalSettingsMenuId) {
		if (!m_menuOrder.contains(btn)) {
			m_ui->stackedWidget->addWidget(m_menuList[-id]);
			m_menuOrder.push_back(btn);
		} else {
			m_menuOrder.removeOne(btn);
			m_menuOrder.push_back(btn);
		}
	}

	if (checked) {
		settingsPanelUpdate(id);
	}

	m_ui->widgetMenuHAnim->toggleMenu(checked);
}

void ToolView::settingsPanelUpdate(int id)
{
	if (id > 0) {
		m_ui->stackedWidget->setCurrentIndex(0);
	} else {
		m_ui->stackedWidget->setCurrentWidget(m_menuList[-id]);
	}

	for (int i = 0; i < m_ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;
		bool visible = false;

		if (i == m_ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
			visible = true;
		}
		QWidget* widget = m_ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
		widget->setVisible(visible);
	}
	m_ui->stackedWidget->adjustSize();
}

void ToolView::buildChannelsContainer(ChannelManager* cm, ChannelsPositionEnum position)
{
	connect(cm, &ChannelManager::channelManagerToggle, this, [=](bool toggled){
		m_ui->widgetVerticalChannels->toggleMenu(toggled);
	});
	connect(cm, &ChannelManager::configureAddBtn, this, &ToolView::configureAddMathBtn);

	connect(this, &ToolView::changeParent, cm, &ChannelManager::changeParent);
	connect(cm, &ChannelManager::positionChanged, this, [=](ChannelsPositionEnum position) {
		if (position == ChannelsPositionEnum::VERTICAL) {
			m_ui->widgetHorizontalChannels->setVisible(false);
			m_ui->widgetVerticalChannels->setVisible(true);

			if (!m_ui->widgetMenuBtns->isVisible()) {
				m_ui->widgetFooter->setVisible(false);
			}

			m_ui->widgetHorizontalChannels->layout()->setSpacing(0);
			m_ui->widgetHorizontalChannels->layout()->setMargin(0);

			Q_EMIT changeParent(m_ui->widgetVerticalChannelsContainer);
		} else {
			m_ui->widgetVerticalChannels->setVisible(false);
			m_ui->widgetFooter->setVisible(true);
			m_ui->widgetHorizontalChannels->setVisible(true);

			Q_EMIT changeParent(m_ui->widgetHorizontalChannelsContainer);
		}
	});

	// if it's horizontal, will be later updated (needs rework)
	m_ui->widgetVerticalChannels->setVisible(true);
	cm->build(m_ui->widgetVerticalChannelsContainer);

}

QDockWidget* ToolView::createDetachableMenu(QWidget* menu, int& id)
{
	QMainWindow* subWindow = new QMainWindow(this);
	QDockWidget* docker = new QDockWidget(subWindow);
	docker->setFeatures(docker->features() & ~QDockWidget::DockWidgetClosable);
	docker->setAllowedAreas(Qt::DockWidgetArea::NoDockWidgetArea);
	subWindow->addDockWidget(Qt::RightDockWidgetArea, docker);
	docker->setWidget(menu);

#ifdef SETTINGS_MENU_BAR_ENABLED
	DockerUtils::configureTopBar(docker);
#endif

	m_ui->stackedWidget->addWidget(subWindow);

	id = getNewID();
	m_menuList[id] = subWindow;

	return docker;
}

QDockWidget *ToolView::createDockableWidget(QWidget *widget, const QString& dockerName)
{
	QDockWidget* docker = new QDockWidget(m_centralMainWindow);
	docker->setWindowTitle(dockerName);
	docker->setFeatures(docker->features() & ~QDockWidget::DockWidgetClosable);
	docker->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	docker->setWidget(widget);

	// workaround, allows dockWidgets movement
	widget->setMinimumHeight(50);
	widget->setMinimumWidth(50);

#ifdef PLOT_MENU_BAR_ENABLED
	DockerUtils::configureTopBar(docker);
#endif
	return docker;
}

void ToolView::configureAddMathBtn(QWidget* menu, bool dockable)
{
	ChannelManager* cm = static_cast<ChannelManager*>(QObject::sender());
	CustomPushButton* addBtn = cm->getAddChannelBtn();
	int id;

	if (dockable) {
		QDockWidget* docker = this->createDetachableMenu(menu, id);

		connect(docker, &QDockWidget::topLevelChanged, addBtn, [=](bool topLevel) {
			addBtn->setChecked(!topLevel);
			addBtn->setDisabled(topLevel);

			if (topLevel) {
				m_menuOrder.removeOne(addBtn);
			}
		});
	} else {
		m_ui->stackedWidget->addWidget(menu);
		id = getNewID();
		m_menuList[id] = dynamic_cast<gui::GenericMenu *>(menu);
	}

	m_group.addButton(addBtn);
	addBtn->setProperty("id", QVariant(-id));

	connect(addBtn, &CustomPushButton::toggled, this, &ToolView::triggerRightMenuToggle);
	connect(addBtn, &CustomPushButton::toggled, m_ui->widgetSettingsPairBtns->getSettingsBtn(),
		&QPushButton::setChecked);
}

ChannelWidget* ToolView::buildNewChannel(ChannelManager* channelManager, GenericMenu* menu, bool dockable, int chId,
					 bool deletable, bool simplefied, QColor color, const QString& fullName,
					 const QString& shortName)
{
	if (chId == -1) {
		chId = getNewID();
	}

	ChannelWidget* ch = channelManager->buildNewChannel(chId, deletable, simplefied, color, fullName, shortName);
	int id;

	if (dockable) {
		QDockWidget* docker = this->createDetachableMenu(menu, id);

		connect(docker, &QDockWidget::topLevelChanged, ch->menuButton(), [=](bool topLevel) {
			CustomPushButton* btn = static_cast<CustomPushButton*>(ch->menuButton());
			btn->setChecked(!topLevel);
			btn->setDisabled(topLevel);

			ch->setMenuFloating(topLevel);

			if (topLevel) {
				m_menuOrder.removeOne(btn);
			}
		});

		if (deletable) {
			connect(ch, &ChannelWidget::deleteClicked, this, [=]() { docker->close(); });
		}

	} else {
		id = chId;
		m_menuList[chId] = menu;
	}

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

		// mirror menu btn
		menu->setMenuButton(toggled);
	});

	connect(menu, &GenericMenu::enableBtnToggled, [=](bool toggled) { ch->enableButton()->setChecked(toggled); });

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

scopy::MenuHAnim* ToolView::addMenuToStack()
{
	return m_ui->widgetMenuHAnim;
}

void ToolView::buildChannelGroup(ChannelManager* channelManager, ChannelWidget* mainChannel, std::vector<ChannelWidget*> channelGroup)
{
	for (ChannelWidget* ch : channelGroup) {
		channelManager->setChannelAlignment(ch,Qt::AlignLeft);
	}
	mainChannel->setIsMainChannel(true);
	channelManager->setChannelAlignment(mainChannel,Qt::AlignLeft);

	connect(mainChannel, &ChannelWidget::enabled,this, [=](){
		for (ChannelWidget* ch : channelGroup) {
			if (mainChannel->toggleChannelsButton()->isChecked()) {
				ch->show();
			} else {
				ch->hide();
			}
		}
	});
}

int ToolView::getNewID()
{
	return m_nextMenuIndex++;
}

CustomMenuButton *ToolView::buildNewInstrumentMenu(GenericMenu* menu, bool dockable, const QString& name, bool checkBoxVisible,
				      bool checkBoxChecked)
{
	m_ui->widgetFooter->setVisible(true);
	m_ui->widgetMenuBtns->setVisible(true);

	CustomMenuButton* btn = new CustomMenuButton(name, checkBoxVisible, checkBoxChecked);
	int id;

	if (dockable) {
		QDockWidget* docker = this->createDetachableMenu(menu, id);

		connect(docker, &QDockWidget::topLevelChanged, btn, [=](bool topLevel) {
			btn->getBtn()->setChecked(!topLevel);
			btn->getBtn()->setDisabled(topLevel);

			btn->setMenuFloating(topLevel);

			if (topLevel) {
				m_menuOrder.removeOne(btn->getBtn());
			}
		});

	} else {
		m_ui->stackedWidget->addWidget(menu);
		id = getNewID();
		m_menuList[id] = menu;
	}

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
		// mirror menu btn
		menu->setMenuButton(toggled);
	});

	// what is this for?
	connect(menu, &GenericMenu::enableBtnToggled, [=](bool toggled) { btn->getCheckBox()->setChecked(toggled); });

	if ((checkBoxVisible && checkBoxChecked) || !checkBoxVisible) {
		m_menuOrder.push_back(btn->getBtn());
	}

	return btn;
}

void ToolView::addFixedCentralWidget(QWidget *widget, int row, int column, int rowspan, int columnspan)
{
	if (row == -1 || column == -1) {
			m_ui->gridWidgetCentral->addWidget(widget);
	} else {
		if (rowspan == -1 || columnspan == -1) {
			m_ui->gridWidgetCentral->addWidget(widget, row, column);
		} else {
			m_ui->gridWidgetCentral->addWidget(widget, row, column, rowspan, columnspan);
		}
	}
}

int ToolView::addDockableCentralWidget(QWidget *widget, Qt::DockWidgetArea area, const QString &dockerName)
{
	QDockWidget* docker = this->createDockableWidget(widget, dockerName);

	m_centralMainWindow->addDockWidget(area, docker);
	m_docksList.append(docker);

	return m_docksList.size() - 1;
}

void ToolView::addPlotInfoWidget(QWidget *widget)
{
	m_ui->widgetPlotInfo->layout()->addWidget(widget);
}

void ToolView::setWidgetVisibility(int widgetId, bool visible)
{
	if (visible) {
		m_docksList.at(widgetId)->show();
	} else {
		m_docksList.at(widgetId)->hide();
	}
}

bool ToolView::isWidgetHidden(int widgetId)
{
	return  m_docksList.at(widgetId)->isHidden();
}

void ToolView::setHeaderVisibility(bool visible)
{
	m_ui->widgetHeader->setVisible(visible);
}

QDockWidget *ToolView::addDockableTabbedWidget(QWidget* plot, const QString &dockerName)
{
	QDockWidget* docker = DockerUtils::createDockWidget(m_centralMainWindow, plot, dockerName);
	m_centralMainWindow->addDockWidget(Qt::LeftDockWidgetArea, docker);

	return docker;
}

int ToolView::addFixedTabbedWidget(QWidget *widget, const QString& title, int plotId, int row, int column, int rowspan, int columnspan)
{
	if(m_centralFixedWidgets.size() > plotId && plotId != -1) {
		// adding widget to an existing TabWidget

		QTabWidget* tabWidget = static_cast<QTabWidget*>(m_centralFixedWidgets.at(plotId));
		tabWidget->addTab(widget, title);

		return plotId;

	} else {
		// creating new TabWidget

		QTabWidget* tabWidget = new QTabWidget(m_ui->widgetCentral);
		tabWidget->addTab(widget, title);
		m_centralFixedWidgets.append(tabWidget);

		this->addFixedCentralWidget(tabWidget, row, column, rowspan, columnspan);

		return m_centralFixedWidgets.size() - 1;
	}
}

void ToolView::setGeneralSettingsMenu(QWidget* menu, bool dockable)
{
	CustomPushButton* generalSettingsBtn = m_ui->widgetSettingsPairBtns->getGeneralSettingsBtn();

	if (dockable) {
		QDockWidget* docker = this->createDetachableMenu(menu, m_generalSettingsMenuId);

		connect(docker, &QDockWidget::topLevelChanged, generalSettingsBtn, [=](bool topLevel) {
			generalSettingsBtn->setChecked(!topLevel);
			generalSettingsBtn->setDisabled(topLevel);
		});
	} else {
		m_ui->stackedWidget->addWidget(menu);
		m_generalSettingsMenuId = getNewID();
		m_menuList[m_generalSettingsMenuId] = menu;
	}

	generalSettingsBtn->setProperty("id", QVariant(-m_generalSettingsMenuId));
	m_group.addButton(generalSettingsBtn);

	connect(generalSettingsBtn, &CustomPushButton::toggled, this, [=](bool toggled) {
		triggerRightMenuToggle(toggled);
		if (toggled) {
			m_ui->widgetSettingsPairBtns->getSettingsBtn()->setChecked(!toggled);
		}
	});

	if (!getGeneralSettingsBtn()->isVisible()) {
		getGeneralSettingsBtn()->setVisible(true);
	}
}

void ToolView::setFixedMenu(QWidget* menu, bool dockable)
{
	int id;

	if (dockable) {
		QDockWidget* docker = this->createDetachableMenu(menu, id);

		connect(docker, &QDockWidget::topLevelChanged,
			[=](bool topLevel) { m_ui->widgetMenuHAnim->toggleMenu(!topLevel); });
	} else {
		m_ui->stackedWidget->addWidget(menu);
		id = getNewID();
	}

	settingsPanelUpdate(-id);
	m_ui->widgetMenuHAnim->toggleMenu(true);
}

QWidget* ToolView::getTopExtraWidget() { return m_ui->widgetTopExtra; }

void ToolView::setVisibleTopExtraWidget(bool visible) { m_ui->widgetTopExtra->setVisible(visible); }

void ToolView::addTopExtraWidget(QWidget* widget) { m_ui->widgetTopExtra->layout()->addWidget(widget); }

QWidget* ToolView::getBottomExtraWidget() { return m_ui->widgetBottomExtra; }

void ToolView::setVisibleBottomExtraWidget(bool visible) { m_ui->widgetBottomExtra->setVisible(visible); }

void ToolView::addBottomExtraWidget(QWidget* widget) { m_ui->widgetBottomExtra->layout()->addWidget(widget); }

QWidget* ToolView::getCentralWidget() { return m_ui->widgetCentral; }

QStackedWidget* ToolView::getStackedWidget() { return m_ui->stackedWidget; }

void ToolView::setStackedWidget(QStackedWidget* sw)
{
	m_ui->stackedWidget = sw;
}

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
