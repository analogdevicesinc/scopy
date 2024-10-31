#include "plugin.h"
#include "toolmenuentry.h"
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <menucombo.h>
#include <QScrollArea>

class MainWidget;
class PluginManager;

class PDKWindow : public QMainWindow
{
	Q_OBJECT
public:
	PDKWindow(QWidget *parent = nullptr);
	~PDKWindow();

public Q_SLOTS:
	void onConnect();
	void onDisconnect();
	void onInit();
	void onDeinit();
	void onLoad();
	void onUnload();
Q_SIGNALS:
	void sigLabelTextUpdated(std::string_view);

private:
	QTabWidget *m_tabWidget;
	QWidget *m_prefPage;
	QList<QWidget *> m_toolList;
	QList<QWidget *> m_loadedTools;
	QLabel *m_aboutPage = nullptr;
	MainWidget *m_mainWidget = nullptr;

	void initMainWindow();
	void updateLabelText();
	void initPreferencesPage();
	void addPluginPrefPage();
	void removePluginPrefPage();
	QLabel *createTabLabel(QString name);
	QWidget *addHorizontalTab(QWidget *w, QLabel *lbl, bool tabEnabled = true);
	QWidget *buildSaveSessionPreference();
	QWidget *generalPreferences();
	void initGeneralPreferences();
};

class MainWidget : public QWidget
{
	Q_OBJECT
public:
	MainWidget(QWidget *parent);
	~MainWidget();

	QString pluginAbout();
	QWidget *pluginPrefPage();
	QList<scopy::ToolMenuEntry *> getPluginTools();

Q_SIGNALS:
	void connected();
	void disconnected();
	void init();
	void deinit();
	void loaded();
	void unloaded();

private Q_SLOTS:
	void onConnect();
	void onDisconnect();
	void deinitialize();
	void onInit();
	void onLoad();
	void onUnload();
	void browseFile(QLineEdit *pluginPathEdit);
	void deletePluginManager();

private:
	void unloadInfoPage();
	void loadInfoPage();
	void changeVisibility(QPushButton *btn1, QPushButton *btn2);
	void enableBrowseInit(bool en);
	void enableLoadConn(bool en);
	void updateStatusLbl(QString msg);
	void loadAvailCat();
	bool validConnection(QString uri, QString cat);
	bool isCompatible(QString uri, QString cat);

	PluginManager *m_pluginManager;
	QLabel *m_statusLbl;
	QLineEdit *m_uriEdit;
	QLineEdit *m_pluginPathEdit;
	QPushButton *m_browseBtn;
	QPushButton *m_initBtn;
	QPushButton *m_loadBtn;
	QPushButton *m_unloadBtn;
	QPushButton *m_connBtn;
	QPushButton *m_disconnBtn;

	QWidget *m_pluginIcon;
	scopy::MenuCombo *m_deviceTypeCb;
	QScrollArea *m_scrollArea;
	QString m_currentPlugin;
	enum State
	{
		Unloaded,
		Loaded,
		Connected
	} m_state;
};

class PluginManager : public QObject
{
	Q_OBJECT
public:
	PluginManager(QString pluginPath, QObject *parent = nullptr);
	~PluginManager();

	scopy::Plugin *plugin() const;
	bool pluginCompatibility(QString param, QString category);

Q_SIGNALS:
	void requestTool(QString toolId);

private:
	void initPlugin();
	void loadPlugin(QString file);

	scopy::Plugin *m_plugin;
	bool m_loaded;
};

class ConnectionStrategy : public QObject
{
	Q_OBJECT
public:
	ConnectionStrategy(QString uri, QObject *parent = nullptr);
	~ConnectionStrategy();
	bool validConnection(QString cat);

private:
	bool iioConn();
	bool testConn();
	QString m_uri;
};
