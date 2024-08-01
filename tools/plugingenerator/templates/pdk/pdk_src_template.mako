#include "pdkwindow.h"
#include <QApplication>
#include <QSettings>
#include <iostream>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QButtonGroup>
#include <QFileDialog>
#include <QPluginLoader>
#include <QJsonArray>
#include <preferences.h>
#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include "gui/stylehelper.h"
#include "gui/preferenceshelper.h"
#include "pdk-util_config.h"
#include <QDesktopServices>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy-PDK");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	app.setStyleSheet(Util::loadStylesheetFromFile(":/default.qss"));
	PDKWindow test;
	test.show();
	int ret = app.exec();
	if(ret == 0) {
		qInfo() << "PDK support finished successfully!";
	}
	return ret;
}

PDKWindow::PDKWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	setMinimumSize(1280, 720);
	layout()->setMargin(9);
	layout()->setSpacing(6);
	scopy::StyleHelper::GetInstance()->initColorMap();
	m_mainWidget = new MainWidget(this);
	m_aboutPage = new QLabel(this);
	m_aboutPage->setWordWrap(true);
	m_aboutPage->setAlignment(Qt::AlignTop);
	initMainWindow();
	initPreferencesPage();

	addHorizontalTab(m_mainWidget, createTabLabel("Main"));
	addHorizontalTab(m_aboutPage, createTabLabel("About"));
	addHorizontalTab(m_prefPage, createTabLabel("Preferences"));
	connect(m_mainWidget, &MainWidget::connected, this, &PDKWindow::onConnect);
	connect(m_mainWidget, &MainWidget::disconnected, this, &PDKWindow::onDisconnect);
	connect(m_mainWidget, &MainWidget::init, this, &PDKWindow::onInit);
	connect(m_mainWidget, &MainWidget::deinit, this, &PDKWindow::onDeinit);
	connect(m_mainWidget, &MainWidget::loaded, this, &PDKWindow::onLoad);
	connect(m_mainWidget, &MainWidget::unloaded, this, &PDKWindow::onUnload);
}

PDKWindow::~PDKWindow()
{
	if(m_mainWidget) {
		delete m_mainWidget;
		m_mainWidget = nullptr;
	}
}

void PDKWindow::onConnect()
{
	if(!m_loadedTools.isEmpty()) {
		onUnload();
	}
	QList<scopy::ToolMenuEntry *> tools = m_mainWidget->getPluginTools();
	for(const scopy::ToolMenuEntry *t : qAsConst(tools)) {
		if(!t->tool())
			continue;
		QLabel *lbl = createTabLabel(t->name());
		QWidget *tab = addHorizontalTab(t->tool(), lbl);
		connect(t, &scopy::ToolMenuEntry::updateToolEntry, this, [lbl, t]() {
			if(lbl->text().compare(t->name()) != 0) {
				lbl->setText(t->name());
			}
		});
		m_toolList.append(tab);
	}
}

void PDKWindow::onDisconnect()
{
	QList<scopy::ToolMenuEntry *> tools = m_mainWidget->getPluginTools();
	for(const scopy::ToolMenuEntry *t : qAsConst(tools)) {
		t->disconnect(this);
	}
	for(auto t : qAsConst(m_toolList)) {
		int idx = m_tabWidget->indexOf(t);
		m_tabWidget->removeTab(idx);
		m_toolList.removeOne(t);
	}
	onLoad();
}

void PDKWindow::onInit()
{
	addPluginPrefPage();
	m_aboutPage->clear();
	m_aboutPage->setText(m_mainWidget->pluginAbout());
}

void PDKWindow::onDeinit()
{
	m_aboutPage->clear();
	removePluginPrefPage();
}

void PDKWindow::onLoad()
{
	QList<scopy::ToolMenuEntry *> tools = m_mainWidget->getPluginTools();
	for(const scopy::ToolMenuEntry *t : qAsConst(tools)) {
		if(t->name().isEmpty())
			continue;
		QLabel *lbl = createTabLabel(t->name());
		QWidget *tab = addHorizontalTab(new QWidget(), lbl, false);
		m_loadedTools.append(tab);
	}
}

void PDKWindow::onUnload()
{
	if(m_loadedTools.isEmpty())
		return;
	for(auto t : qAsConst(m_loadedTools)) {
		int idx = m_tabWidget->indexOf(t);
		m_tabWidget->removeTab(idx);
		m_loadedTools.removeOne(t);
	}
}

QWidget *PDKWindow::buildSaveSessionPreference()
{
	scopy::Preferences *p = scopy::Preferences::GetInstance();
	QWidget *w = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(w);
	lay->setMargin(0);

	lay->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(p, "general_save_session",
								       "Save/Load Scopy session", this));
	lay->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(new QLabel("Settings files location ", this));
	QPushButton *navigateBtn = new QPushButton("Open", this);
	scopy::StyleHelper::BlueButton(navigateBtn, "navigateBtn");
	navigateBtn->setMaximumWidth(80);
	connect(navigateBtn, &QPushButton::clicked, this,
		[=]() { QDesktopServices::openUrl(scopy::config::settingsFolderPath()); });
	lay->addWidget(navigateBtn);
	return w;
}

QWidget *PDKWindow::generalPreferences()
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	initGeneralPreferences();
	scopy::Preferences *p = scopy::Preferences::GetInstance();

	lay->setMargin(0);
	lay->setSpacing(10);
	page->setLayout(lay);

	// General preferences
	scopy::MenuSectionWidget *generalWidget = new scopy::MenuSectionWidget(page);
	scopy::MenuCollapseSection *generalSection =
		new scopy::MenuCollapseSection("General", scopy::MenuCollapseSection::MHCW_NONE, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(buildSaveSessionPreference());
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_save_attached", "Save/Load tool attached state", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_doubleclick_attach", "Doubleclick to attach/detach tool", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_doubleclick_ctrl_opens_menu", "Doubleclick control buttons to open menu", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_use_opengl", "Enable OpenGL plotting", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_use_animations", "Enable menu animations", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_check_online_version", "Enable automatic online check for updates.", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_show_status_bar", "Enable the status bar for displaying important messages.",
		generalSection));
	generalSection->contentLayout()->addWidget(
		scopy::PreferencesHelper::addPreferenceCheckBox(p, "show_grid", "Show Grid", generalSection));
	generalSection->contentLayout()->addWidget(
		scopy::PreferencesHelper::addPreferenceCheckBox(p, "show_graticule", "Show Graticule", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "iiowidgets_use_lazy_loading", "Use Lazy Loading", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "plugins_use_debugger_v2", "Use Debugger V2 plugin", generalSection));
	generalSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCombo(
		p, "general_theme", "Theme", {"default", "light"}, generalSection));

	// Debug preferences
	scopy::MenuSectionWidget *debugWidget = new scopy::MenuSectionWidget(page);
	scopy::MenuCollapseSection *debugSection =
		new scopy::MenuCollapseSection("Debug", scopy::MenuCollapseSection::MHCW_NONE, debugWidget);
	debugWidget->contentLayout()->setSpacing(10);
	debugWidget->contentLayout()->addWidget(debugSection);
	debugSection->contentLayout()->setSpacing(10);
	lay->addWidget(debugWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	debugSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCheckBox(
		p, "general_show_plot_fps", "Show plot FPS", debugSection));
	debugSection->contentLayout()->addWidget(scopy::PreferencesHelper::addPreferenceCombo(
		p, "general_plot_target_fps", "Plot target FPS", {"15", "20", "30", "60"}, debugSection));

	return page;
}

void PDKWindow::initGeneralPreferences()
{
	scopy::Preferences *p = scopy::Preferences::GetInstance();
	p->setPreferencesFilename(PATH_TO_INI);
	p->load();
	p->init("general_first_run", true);
	p->init("general_save_session", true);
	p->init("general_save_attached", true);
	p->init("general_doubleclick_attach", true);
#if defined(__arm__)
	p->init("general_use_opengl", false);
#else
	p->init("general_use_opengl", true);
#endif
	p->init("general_use_animations", true);
	p->init("general_theme", "default");
	p->init("general_language", "en");
	p->init("show_grid", true);
	p->init("show_graticule", false);
	p->init("iiowidgets_use_lazy_loading", true);
	p->init("plugins_use_debugger_v2", true);
	p->init("general_plot_target_fps", "60");
	p->init("general_show_plot_fps", true);
	p->init("general_use_native_dialogs", true);
	p->init("general_additional_plugin_path", "");
	p->init("general_load_decoders", true);
	p->init("general_doubleclick_ctrl_opens_menu", true);
	p->init("general_check_online_version", false);
	p->init("general_show_status_bar", true);
}

void PDKWindow::initMainWindow()
{
	QWidget *centralWidget = new QWidget(this);
	centralWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	centralWidget->setMinimumSize(1280, 720);
	setCentralWidget(centralWidget);
	QHBoxLayout *lay = new QHBoxLayout(centralWidget);
	lay->setMargin(9);
	lay->setSpacing(6);

	m_tabWidget = new QTabWidget(centralWidget);
	m_tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_tabWidget->setTabPosition(QTabWidget::TabPosition::West);
	scopy::StyleHelper::BackgroundPage(m_tabWidget, "pdkTable");

	lay->addWidget(m_tabWidget);
}

QWidget *PDKWindow::addHorizontalTab(QWidget *w, QLabel *lbl, bool tabEnabled)
{
	QWidget *pane = new QWidget(m_tabWidget);
	pane->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *lay = new QHBoxLayout(pane);
	lay->setMargin(10);
	pane->setLayout(lay);

	QScrollArea *scrollArea = new QScrollArea(pane);
	scrollArea->setWidget(w);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lay->addWidget(scrollArea);

	// Hackish - so we don't override paint event
	int tabIdx = m_tabWidget->addTab(pane, "");
	m_tabWidget->setTabEnabled(tabIdx, tabEnabled);

	QTabBar *tabbar = m_tabWidget->tabBar();
	tabbar->setTabButton(tabbar->count() - 1, QTabBar::LeftSide, lbl);
	tabbar->tabButton(tabbar->count() - 1, QTabBar::LeftSide)->setEnabled(tabEnabled);
	return pane;
}
void PDKWindow::initPreferencesPage()
{
	m_prefPage = new QWidget(this);
	QGridLayout *lay = new QGridLayout(m_prefPage);
	lay->setSpacing(6);
	lay->setMargin(0);

	scopy::MenuSectionWidget *generalWidget = new scopy::MenuSectionWidget(m_prefPage);
	scopy::MenuCollapseSection *generalSection = new scopy::MenuCollapseSection(
		"Scopy preferences", scopy::MenuCollapseSection::MHCW_NONE, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	generalSection->contentLayout()->addWidget(generalPreferences());

	lay->addWidget(generalWidget, 0, 0);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 2, 0);
}

void PDKWindow::addPluginPrefPage()
{
	QWidget *pluginPref = m_mainWidget->pluginPrefPage();
	if(pluginPref) {
		QGridLayout *lay = dynamic_cast<QGridLayout *>(m_prefPage->layout());
		if(!lay)
			return;
		scopy::MenuSectionWidget *pluginWidget = new scopy::MenuSectionWidget(m_prefPage);
		scopy::MenuCollapseSection *pluginSection = new scopy::MenuCollapseSection(
			"Plugin preferences", scopy::MenuCollapseSection::MHCW_NONE, pluginWidget);
		pluginWidget->contentLayout()->setSpacing(10);
		pluginWidget->contentLayout()->addWidget(pluginSection);
		pluginSection->contentLayout()->setSpacing(10);
		pluginSection->contentLayout()->addWidget(pluginPref);
		lay->addWidget(pluginWidget, 1, 0);
	}
}

void PDKWindow::removePluginPrefPage()
{
	QGridLayout *lay = dynamic_cast<QGridLayout *>(m_prefPage->layout());
	QLayoutItem *it = lay->itemAtPosition(1, 0);
	if(!it) {
		return;
	}
	QWidget *pluginPref = it->widget();
	if(pluginPref) {
		m_prefPage->layout()->removeWidget(pluginPref);
		delete pluginPref;
	}
}

QLabel *PDKWindow::createTabLabel(QString name)
{
	QLabel *lbl = new QLabel();
	scopy::StyleHelper::TabWidgetLabel(lbl, "tabWidgetLabel");
	lbl->setText(name);
	return lbl;
}

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
	, m_pluginManager(nullptr)
{
	m_state = State::Unloaded;
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setMinimumWidth(1024);
	QGridLayout *lay = new QGridLayout(this);
	lay->setMargin(9);
	lay->setSpacing(6);

	m_statusLbl = new QLabel(this);
	scopy::StyleHelper::WarningLabel(m_statusLbl);
	m_statusLbl->setWordWrap(true);

	// browse section
	m_browseBtn = new QPushButton("Browse", this);
	scopy::StyleHelper::BlueButton(m_browseBtn);
	m_browseBtn->setFixedWidth(128);
	m_browseBtn->setFixedHeight(40);

	m_pluginPathEdit = new QLineEdit(this);
	m_pluginPathEdit->setText(PLUGIN_INSTALL_PATH);
	m_pluginPathEdit->setPlaceholderText(PLUGIN_INSTALL_PATH);
	connect(m_browseBtn, &QPushButton::clicked, this, [this]() {
		browseFile(m_pluginPathEdit);
		enableLoadConn(false);
	});

	// init
	m_initBtn = new QPushButton("Init", this);
	scopy::StyleHelper::BlueButton(m_initBtn);
	m_initBtn->setFixedWidth(128);
	m_initBtn->setFixedHeight(40);
	connect(m_initBtn, &QPushButton::clicked, this, &MainWidget::onInit);

	// load btn
	QWidget *loadBtns = new QWidget(this);
	loadBtns->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	loadBtns->setLayout(new QVBoxLayout(loadBtns));
	loadBtns->layout()->setSpacing(0);
	loadBtns->layout()->setMargin(0);

	m_loadBtn = new QPushButton("Load", this);
	scopy::StyleHelper::BlueButton(m_loadBtn);
	m_loadBtn->setFixedWidth(128);
	m_loadBtn->setFixedHeight(40);
	m_loadBtn->setEnabled(false);
	connect(m_loadBtn, &QPushButton::clicked, this, &MainWidget::onLoad);

	m_unloadBtn = new QPushButton("Unload", this);
	scopy::StyleHelper::BlueButton(m_unloadBtn);
	m_unloadBtn->setFixedWidth(128);
	m_unloadBtn->setFixedHeight(40);
	m_unloadBtn->setVisible(false);
	connect(m_unloadBtn, &QPushButton::clicked, this, &MainWidget::onUnload);

	loadBtns->layout()->addWidget(m_loadBtn);
	loadBtns->layout()->addWidget(m_unloadBtn);

	// connect section
	m_uriEdit = new QLineEdit(this);
	m_uriEdit->setPlaceholderText("URI");
	m_uriEdit->setText("ip:127.0.0.1");

	m_deviceTypeCb = new scopy::MenuCombo("category", this);
	m_deviceTypeCb->setFixedHeight(40);

	QWidget *connBtns = new QWidget(this);
	connBtns->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	connBtns->setLayout(new QVBoxLayout(connBtns));
	connBtns->layout()->setSpacing(0);
	connBtns->layout()->setMargin(0);

	m_connBtn = new QPushButton("Connect", this);
	scopy::StyleHelper::BlueButton(m_connBtn);
	m_connBtn->setFixedWidth(128);
	m_connBtn->setFixedHeight(40);
	m_connBtn->setEnabled(false);
	connect(m_connBtn, &QPushButton::clicked, this, &MainWidget::onConnect);

	m_disconnBtn = new QPushButton("Disconnect", this);
	scopy::StyleHelper::BlueButton(m_disconnBtn);
	m_disconnBtn->setFixedWidth(128);
	m_disconnBtn->setFixedHeight(40);
	m_disconnBtn->setVisible(false);
	connect(m_disconnBtn, &QPushButton::clicked, this, &MainWidget::onDisconnect);

	connBtns->layout()->addWidget(m_connBtn);
	connBtns->layout()->addWidget(m_disconnBtn);

	// plugin info
	QWidget *pluginPage = new QWidget(this);
	pluginPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pluginPage->setLayout(new QVBoxLayout(pluginPage));
	pluginPage->layout()->setMargin(0);

	m_scrollArea = new QScrollArea(pluginPage);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	pluginPage->layout()->addWidget(m_scrollArea);

	m_pluginIcon = new QWidget(this);
	m_pluginIcon->setLayout(new QHBoxLayout(m_pluginIcon));
	m_pluginIcon->layout()->setMargin(0);
	m_pluginIcon->setFixedSize(100, 100);

	lay->addWidget(m_pluginPathEdit, 0, 0);
	lay->addWidget(m_deviceTypeCb, 1, 0);
	lay->addWidget(m_uriEdit, 2, 0);
	lay->addWidget(m_statusLbl, 3, 0);
	lay->addWidget(pluginPage, 4, 0);

	lay->addWidget(m_browseBtn, 0, 1, Qt::AlignTop);
	lay->addWidget(m_initBtn, 1, 1, Qt::AlignTop);
	lay->addWidget(loadBtns, 2, 1, Qt::AlignTop);
	lay->addWidget(connBtns, 3, 1, Qt::AlignTop);
	lay->addWidget(m_pluginIcon, 4, 1, Qt::AlignCenter | Qt::AlignBottom);
}

MainWidget::~MainWidget()
{
	if(m_state == State::Connected) {
		onDisconnect();
	}
	if(m_state == State::Loaded) {
		onUnload();
	}
	if(!m_currentPlugin.isEmpty()) {
		deinitialize();
	}
}

QString MainWidget::pluginAbout()
{
	if(!m_pluginManager || !m_pluginManager->plugin())
		return "";
	return m_pluginManager->plugin()->about();
}

QWidget *MainWidget::pluginPrefPage()
{
	QWidget *prefPage = nullptr;
	m_pluginManager->plugin()->initPreferences();
	if(m_pluginManager->plugin()->loadPreferencesPage())
		prefPage = m_pluginManager->plugin()->preferencesPage();

	return prefPage;
}

QList<scopy::ToolMenuEntry *> MainWidget::getPluginTools() { return m_pluginManager->plugin()->toolList(); }

void MainWidget::onConnect()
{
	if(m_state == State::Unloaded)
		onLoad();
	if(m_state != State::Loaded)
		return;
	if(!m_pluginManager->plugin()->onConnect()) {
		updateStatusLbl("Connection failed!");
		return;
	}
	changeVisibility(m_disconnBtn, m_connBtn);
	enableBrowseInit(false);
	m_unloadBtn->setEnabled(false);
	m_state = State::Connected;
	updateStatusLbl("");
	Q_EMIT connected();
}

void MainWidget::onDisconnect()
{
	m_pluginManager->plugin()->onDisconnect();
	changeVisibility(m_disconnBtn, m_connBtn);
	m_unloadBtn->setEnabled(true);
	m_state = State::Loaded;
	Q_EMIT disconnected();
}

void MainWidget::changeVisibility(QPushButton *btn1, QPushButton *btn2)
{
	btn1->setVisible(!btn1->isVisible());
	btn2->setVisible(!btn2->isVisible());
}

void MainWidget::enableBrowseInit(bool en)
{
	m_initBtn->setEnabled(en);
	m_browseBtn->setEnabled(en);
}

void MainWidget::enableLoadConn(bool en)
{
	m_loadBtn->setEnabled(en);
	m_connBtn->setEnabled(en);
}

void MainWidget::updateStatusLbl(QString msg)
{
	m_statusLbl->clear();
	m_statusLbl->setText(msg);
}

void MainWidget::loadAvailCat()
{
	scopy::Plugin *plugin = m_pluginManager->plugin();
	if(plugin->metadata().isEmpty())
		return;
	if(!plugin->metadata().contains("category")) // plugin metadata does not have category
		return;
	QJsonValue categoryVal = plugin->metadata().value("category");
	if(categoryVal.isString()) // single category
		m_deviceTypeCb->combo()->addItem(categoryVal.toString());
	if(categoryVal.isArray()) { // list category
		QJsonArray catArray = categoryVal.toArray();
		for(const auto &v : catArray) {
			if(!v.isString()) {
				continue;
			}
			m_deviceTypeCb->combo()->addItem(v.toString());
		}
	}
}

void MainWidget::deinitialize()
{
	if(m_pluginManager) {
		QWidget *pluginIcon = m_pluginManager->plugin()->icon();
		if(pluginIcon) {
			m_pluginIcon->layout()->removeWidget(pluginIcon);
			delete pluginIcon;
		}
	}
	m_deviceTypeCb->combo()->clear();
	deletePluginManager();
	Q_EMIT deinit();
}

void MainWidget::onInit()
{
	QString pluginPath = m_pluginPathEdit->text();
	if(pluginPath.compare(m_currentPlugin) == 0 && m_pluginManager) {
		updateStatusLbl("The plugin is already initialized!");
		return;
	}
	deinitialize();
	m_pluginManager = new PluginManager(pluginPath, this);
	if(!m_pluginManager->plugin()) {
		enableLoadConn(false);
		deletePluginManager();
		updateStatusLbl("The plugin couldn't be accessed! Path: " + pluginPath);
		return;
	}
	if(m_pluginManager->plugin()->loadIcon()) {
		m_pluginIcon->layout()->addWidget(m_pluginManager->plugin()->icon());
	}
	m_pluginManager->plugin()->initMetadata();
	loadAvailCat();
	m_currentPlugin = pluginPath;
	enableLoadConn(true);
	updateStatusLbl("");
	Q_EMIT init();
}

void MainWidget::onLoad()
{
	QString uri = m_uriEdit->text();
	QString cat = m_deviceTypeCb->combo()->currentText();
	if(!validConnection(uri, cat) || !isCompatible(uri, cat)) {
		return;
	}
	loadInfoPage();
	enableBrowseInit(false);
	changeVisibility(m_unloadBtn, m_loadBtn);
	m_state = State::Loaded;
	updateStatusLbl("");
	m_deviceTypeCb->setEnabled(false);
	m_uriEdit->setEnabled(false);
	Q_EMIT loaded();
}

void MainWidget::onUnload()
{
	m_pluginManager->plugin()->unload();
	unloadInfoPage();
	enableBrowseInit(true);
	changeVisibility(m_unloadBtn, m_loadBtn);
	m_state = State::Unloaded;
	m_deviceTypeCb->setEnabled(true);
	m_uriEdit->setEnabled(true);
	Q_EMIT unloaded();
}

void MainWidget::browseFile(QLineEdit *pluginPathEdit)
{
	QString filePath =
		QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
					     "All (*);;XML Files (*.xml);;Text Files (*.txt);;BIN Files (*.bin)");
	pluginPathEdit->setText(filePath);
}

void MainWidget::deletePluginManager()
{
	if(m_pluginManager) {
		delete m_pluginManager;
		m_pluginManager = nullptr;
	}
}

void MainWidget::unloadInfoPage()
{
	QWidget *pluginPage = m_pluginManager->plugin()->page();
	if(pluginPage) {
		m_scrollArea->takeWidget();
	}
}

void MainWidget::loadInfoPage()
{
	if(m_pluginManager->plugin()->loadPage()) {
		m_scrollArea->setWidget(m_pluginManager->plugin()->page());
	}
}

bool MainWidget::validConnection(QString uri, QString cat)
{
	ConnectionStrategy *connStrategy = new ConnectionStrategy(uri, this);
	bool validConn = false;
	validConn = connStrategy->validConnection(cat);
	if(!validConn) {
		updateStatusLbl("Cannot connect to URI!");
	}
	connStrategy->deleteLater();
	return validConn;
}

bool MainWidget::isCompatible(QString uri, QString cat)
{
	bool compatible = false;
	compatible = m_pluginManager->pluginCompatibility(uri, cat);
	if(!compatible) {
		updateStatusLbl("The plugin is not compatible with the device or doesn't exist!");
	}
	return compatible;
}

PluginManager::PluginManager(QString pluginPath, QObject *parent)
	: QObject(parent)
	, m_loaded(false)
	, m_plugin(nullptr)
{
	loadPlugin(pluginPath);
}

PluginManager::~PluginManager()
{
	if(m_plugin) {
		delete m_plugin;
		m_plugin = nullptr;
	}
}

void PluginManager::initPlugin()
{
	if(!m_plugin || m_loaded)
		return;
	m_plugin->preload();
	m_plugin->init();
	m_plugin->loadToolList();
	m_plugin->loadExtraButtons();
	m_plugin->postload();
	m_loaded = true;
}

bool PluginManager::pluginCompatibility(QString param, QString category)
{
	initPlugin();
	if(!m_plugin->compatible(param, category))
		return false;
	m_plugin->setParam(param, category);
	m_plugin->setEnabled(true);
	return true;
}

scopy::Plugin *PluginManager::plugin() const { return m_plugin; }

void PluginManager::loadPlugin(QString file)
{
	bool ret;
	scopy::Plugin *original = nullptr;
	scopy::Plugin *clone = nullptr;

	if(!QFile::exists(file))
		return;

	if(!QLibrary::isLibrary(file))
		return;

	QPluginLoader qp(file);
	ret = qp.load();
	if(!ret) {
		qWarning() << "Cannot load library " + qp.fileName() + "- err: " + qp.errorString();
		return;
	}

	QObject *inst = qp.instance();
	if(!inst) {
		qWarning() << "Cannot create QObject instance from loaded library";
		return;
	}

	original = qobject_cast<scopy::Plugin *>(qp.instance());
	if(!original) {
		qWarning() << "Loaded library instance is not a Plugin*";
		return;
	}

	clone = original->clone(this);
	if(!clone) {
		qWarning() << "clone method does not clone the object";
		return;
	}

	QString cloneName;
	cloneName = clone->name();

	if(cloneName == "")
		return;

	m_plugin = clone;
}

ConnectionStrategy::ConnectionStrategy(QString uri, QObject *parent)
	: m_uri(uri)
	, QObject(parent)
{}

ConnectionStrategy::~ConnectionStrategy() {}

bool ConnectionStrategy::validConnection(QString cat)
{
	bool valid = false;
	if(cat.compare("iio") == 0) {
		valid = iioConn();
	} else {
		valid = testConn();
	}
	return valid;
}

bool ConnectionStrategy::iioConn()
{
	iio_context *ctx = iio_create_context_from_uri(m_uri.toStdString().c_str());
	if(!ctx) {
		return false;
	}
	iio_context_destroy(ctx);
	return true;
}

bool ConnectionStrategy::testConn() { return true; }
