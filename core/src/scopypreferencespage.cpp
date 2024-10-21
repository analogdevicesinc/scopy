/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scopypreferencespage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include <QDesktopServices>
#include <QUrl>
#include <QCoreApplication>
#include <QScrollArea>
#include <stylehelper.h>
#include <deviceautoconnect.h>
#include "gui/preferenceshelper.h"
#include <style.h>
#include "application_restarter.h"
#include "smallOnOffSwitch.h"
#include <QDir>
#include <QDebug>
#include <QLoggingCategory>
#include <common/scopyconfig.h>
#include <translationsrepository.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <verticaltabwidget.h>

Q_LOGGING_CATEGORY(CAT_PREFERENCESPAGE, "ScopyPreferencesPage");

using namespace scopy;
ScopyPreferencesPage::ScopyPreferencesPage(QWidget *parent)
	: QWidget(parent)
	, tabWidget(new VerticalTabWidget(this))
	, layout(new QVBoxLayout(this))
{
	initUI();
	initRestartWidget();

	addHorizontalTab(buildGeneralPreferencesPage(), "General");
}

void ScopyPreferencesPage::initUI()
{
	layout->setMargin(0);
	layout->setSpacing(0);
	this->setLayout(layout);
	layout->addWidget(tabWidget);

	StyleHelper::BackgroundPage(tabWidget, "preferencesTable");
	StyleHelper::TabWidgetEastMenu(tabWidget, "preferencesTable");
}

void ScopyPreferencesPage::addHorizontalTab(QWidget *w, QString text)
{
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QWidget *pane = new QWidget();
	pane->setStyleSheet(".QWidget{background-color: " + Style::getAttribute(json::theme::background_subtle) + ";}");
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setMargin(20);
	pane->setLayout(lay);

	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidget(w);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lay->addWidget(scrollArea);
	scrollArea->viewport()->setStyleSheet(
		".QWidget{background-color: " + Style::getAttribute(json::theme::background_subtle) + ";}");
	tabWidget->addTab(pane, text);
}

void ScopyPreferencesPage::initSessionDevices()
{
	QMap<QString, QStringList> prevSession;
	QMap<QString, QVariant> devicesMap = Preferences::get("autoconnect_devices").toMap();
	for(QMap<QString, QVariant>::iterator it = devicesMap.begin(); it != devicesMap.end(); ++it) {
		QStringList plugins = it.value().toString().split(";");
		prevSession[it.key()] = plugins;
	}
	updateSessionDevices(prevSession);
}

void ScopyPreferencesPage::updateSessionDevices(QMap<QString, QStringList> devices)
{
	int btnIdx = m_autoConnectWidget->contentLayout()->indexOf(m_devRefresh);
	QStringList keys = devices.keys();
	for(const QString &uri : qAsConst(keys)) {
		if(!m_connDevices.contains(uri)) {
			QString prefId = uri + "_sticky";
			QCheckBox *devCb = new QCheckBox(uri, m_autoConnectWidget);
			devCb->setObjectName(uri);
			devCb->setChecked(Preferences::get(prefId).toBool());
			m_connDevices[uri] = devCb;
			m_autoConnectWidget->contentLayout()->insertWidget(btnIdx, devCb);
			connect(devCb, &QCheckBox::toggled, this, [=](bool en) {
				if(en) {
					DeviceAutoConnect::addDevice(uri, devices[uri]);
				} else {
					DeviceAutoConnect::removeDevice(uri);
				}
				Preferences::set(prefId, en);
			});
		}
	}
}

ScopyPreferencesPage::~ScopyPreferencesPage() {}

void ScopyPreferencesPage::initRestartWidget()
{
	restartWidget = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);
	lay->setMargin(10);
	restartWidget->setLayout(lay);
	restartWidget->setVisible(false);
	QLabel *lab = new QLabel("An application restart is required for these settings to take effect. ");
	QSpacerItem *space1 = new QSpacerItem(6, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QSpacerItem *space2 = new QSpacerItem(6, 20, QSizePolicy::Preferred, QSizePolicy::Fixed);
	QPushButton *btn = new QPushButton("Restart");
	Style::setStyle(btn, style::properties::button::basicButton, true, true);
	StyleHelper::BackgroundWidget(restartWidget, "restartWidget");
	btn->setFixedWidth(100);

	lay->addWidget(btn);
	lay->addSpacerItem(space2);
	lay->addWidget(lab);
	lay->addSpacerItem(space1);
	layout->addWidget(restartWidget);

	connect(btn, &QPushButton::clicked, btn, []() { ApplicationRestarter::triggerRestart(); });
	connect(Preferences::GetInstance(), &Preferences::restartRequired, this,
		[=]() { restartWidget->setVisible(true); });
}

QWidget *ScopyPreferencesPage::buildSaveSessionPreference()
{
	Preferences *p = Preferences::GetInstance();
	QWidget *w = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(w);
	lay->setMargin(0);

	lay->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "general_save_session", "Save/Load Scopy session", this));
	lay->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(new QLabel("Settings files location ", this));
	QPushButton *navigateBtn = new QPushButton("Open", this);
	Style::setStyle(navigateBtn, style::properties::button::borderButton);
	navigateBtn->setMaximumWidth(80);
	connect(navigateBtn, &QPushButton::clicked, this,
		[=]() { QDesktopServices::openUrl(scopy::config::settingsFolderPath()); });
	lay->addWidget(navigateBtn);
	return w;
}

void ScopyPreferencesPage::removeIniFiles(bool backup)
{
	QString dir = scopy::config::settingsFolderPath();
	QDir loc(dir);
	QFileInfoList plugins = loc.entryInfoList(QDir::Files);
	QStringList settingsFiles;

	for(const QFileInfo &p : plugins) {
		if(p.suffix() == "ini")
			settingsFiles.append(p.absoluteFilePath());
	}
	qInfo(CAT_PREFERENCESPAGE) << "Removing ini files .. ";
	for(auto &&file : settingsFiles) {
		if(backup) {
			QFile(file + ".bak").remove();
			QFile(file).rename(file + ".bak");
			qDebug(CAT_PREFERENCESPAGE) << "Renamed" << file << "to" << file << ".bak";
		} else {
			QFile(file).remove();
			qDebug(CAT_PREFERENCESPAGE) << "Removed" << file;
		}
	}
}

void ScopyPreferencesPage::resetScopyPreferences()
{
	Preferences *p = Preferences::GetInstance();
	removeIniFiles();
	p->clear();
	Q_EMIT Preferences::GetInstance()->restartRequired();
}

QWidget *ScopyPreferencesPage::buildResetScopyDefaultButton()
{
	QWidget *w = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(w);

	QPushButton *resetBtn = new QPushButton("Reset", this);
	Style::setStyle(resetBtn, style::properties::button::borderButton);
	resetBtn->setMaximumWidth(80);
	connect(resetBtn, &QPushButton::clicked, this, &ScopyPreferencesPage::resetScopyPreferences);
	lay->addWidget(resetBtn);
	lay->setMargin(0);
	lay->addSpacerItem(new QSpacerItem(6, 40, QSizePolicy::Preferred, QSizePolicy::Fixed));
	lay->addWidget(new QLabel("Reset to settings and plugins to default"));
	lay->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Expanding, QSizePolicy::Fixed));

	return w;
}

QWidget *ScopyPreferencesPage::buildGeneralPreferencesPage()
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	Preferences *p = Preferences::GetInstance();
	TranslationsRepository *t = scopy::TranslationsRepository::GetInstance();

	lay->setMargin(0);
	lay->setSpacing(10);
	page->setLayout(lay);

	// General preferences
	MenuSectionWidget *generalWidget = new MenuSectionWidget(page);
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(buildSaveSessionPreference());
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_save_attached", "Save/Load tool attached state", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_doubleclick_attach", "Doubleclick to attach/detach tool", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_doubleclick_ctrl_opens_menu", "Doubleclick control buttons to open menu", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_use_opengl", "Enable OpenGL plotting", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_use_animations", "Enable menu animations", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_check_online_version", "Enable automatic online check for updates.", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_show_status_bar", "Enable the status bar for displaying important messages.",
		generalSection));
	generalSection->contentLayout()->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "show_grid", "Show Grid", generalSection));
	generalSection->contentLayout()->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "show_graticule", "Show Graticule", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "iiowidgets_use_lazy_loading", "Use Lazy Loading", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_use_native_dialogs", "Use native dialogs", generalSection));
	QWidget *autoConnectCb = PreferencesHelper::addPreferenceCheckBox(
		p, "autoconnect_previous", "Auto-connect to previous session", generalSection);
	generalSection->contentLayout()->addWidget(autoConnectCb);
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "general_theme", "Theme", Style::GetInstance()->getThemeList(), generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "general_language", "Language", t->getLanguages(), generalSection));
	generalSection->contentLayout()->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "general_connect_to_multiple_devices",
							 "Connect to multiple devices (EXPERIMENTAL)", generalSection));
	generalSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCheckBox(
		p, "general_scan_for_devices", "Regularly scan for new devices", generalSection));

	// Auto-connect
	m_autoConnectWidget = new MenuSectionCollapseWidget("Session devices", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_COMPOSITEWIDGET, page);
	MenuCollapseHeader *autoConnectHeader =
		dynamic_cast<MenuCollapseHeader *>(m_autoConnectWidget->collapseSection()->header());
	autoConnectHeader->headerWidget()->layout()->addWidget(
		new QLabel("At each auto-connect session, it will try to connect to the checked devices"));
	m_autoConnectWidget->contentLayout()->setSpacing(10);
	lay->addWidget(m_autoConnectWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	m_devRefresh = new QPushButton("Refresh", m_autoConnectWidget);
	m_devRefresh->setMaximumWidth(80);
	Style::setStyle(m_devRefresh, style::properties::button::basicButton);
	m_autoConnectWidget->add(m_devRefresh);

	connect(m_devRefresh, &QPushButton::pressed, this, &ScopyPreferencesPage::refreshDevicesPressed);

	// Debug preferences
	MenuSectionWidget *debugWidget = new MenuSectionWidget(page);
	MenuCollapseSection *debugSection = new MenuCollapseSection("Debug", MenuCollapseSection::MHCW_NONE,
								    MenuCollapseSection::MHW_BASEWIDGET, debugWidget);
	debugWidget->contentLayout()->setSpacing(10);
	debugWidget->contentLayout()->addWidget(debugSection);
	debugSection->contentLayout()->setSpacing(10);
	lay->addWidget(debugWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	debugSection->contentLayout()->addWidget(
		PreferencesHelper::addPreferenceCheckBox(p, "general_show_plot_fps", "Show plot FPS", debugSection));
	debugSection->contentLayout()->addWidget(PreferencesHelper::addPreferenceCombo(
		p, "general_plot_target_fps", "Plot target FPS", {"15", "20", "30", "60"}, debugSection));
	debugSection->contentLayout()->addWidget(buildResetScopyDefaultButton());

	return page;
}

#include "moc_scopypreferencespage.cpp"
