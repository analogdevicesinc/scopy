#include "scopymainwindow_api.h"

#include "qapplication.h"

#include <pluginbase/scopyjs.h>
using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCOPY_API, "Scopy_API")

ScopyMainWindow_API::ScopyMainWindow_API(ScopyMainWindow *w)
	: ApiObject()
	, m_w(w)
{}

ScopyMainWindow_API::~ScopyMainWindow_API() {}

void ScopyMainWindow_API::acceptLicense()
{
	if(m_w->license) {
		Q_EMIT m_w->license->getContinueBtn()->clicked();
	}

	if(m_w->checkUpdate) {
		Q_EMIT m_w->checkUpdate->setCheckVersion(false);
	}
}

QString ScopyMainWindow_API::addDevice(QString uri, QString cat, bool async)
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(uri);
	QString devID = "";
	if(conn) {
		Q_ASSERT(m_w->dm != nullptr);
		devID = m_w->dm->createDevice(cat, uri, async);
	} else {
		uri = "ip:" + uri;
		conn = cp->open(uri);
		if(conn) {
			devID = m_w->dm->createDevice(cat, uri, async);
		} else {
			qWarning(CAT_SCOPY_API) << "No device available";
		}
	}
	return devID;
}

QString ScopyMainWindow_API::addDevice(QString uri, QList<QString> plugins, QString cat, bool async)
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(uri);
	QString devID = "";
	if(conn) {
		Q_ASSERT(m_w->dm != nullptr);
		devID = m_w->dm->createDevice(cat, uri, async, plugins);
	} else {
		uri = "ip:" + uri;
		conn = cp->open(uri);
		if(conn) {
			devID = m_w->dm->createDevice(cat, uri, async, plugins);
		} else {
			qWarning(CAT_SCOPY_API) << "No device available";
		}
	}
	return devID;
}

Device *ScopyMainWindow_API::getDevice(int idx)
{
	Q_ASSERT(m_w->dm != nullptr);
	Device *dev = nullptr;
	QList mapKeys = m_w->dm->map.keys();
	std::sort(mapKeys.begin(), mapKeys.end(), sortByUUID);
	if(!mapKeys.isEmpty() && idx < mapKeys.size()) {
		dev = m_w->dm->map[mapKeys[idx]];
	}
	return dev;
}

bool ScopyMainWindow_API::removeDevice(QString uri, QString cat)
{
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(uri);
	Q_ASSERT(m_w->dm != nullptr);
	bool devRemoved = false;
	if(conn) {
		m_w->dm->removeDevice(cat, uri);
		devRemoved = true;
	} else {
		qWarning(CAT_SCOPY_API) << "No device found";
	}
	return devRemoved;
}

bool ScopyMainWindow_API::removeDevice(int idx)
{
	Device *dev = getDevice(idx);
	bool devRemoved = false;
	if(dev) {
		m_w->dm->removeDevice(dev->category(), dev->param());
		devRemoved = true;
	} else {
		qWarning(CAT_SCOPY_API) << "No device found";
	}
	return devRemoved;
}

bool ScopyMainWindow_API::startScan(bool scanState)
{
	ScanButtonController *sbc = new ScanButtonController(m_w->scanCycle, m_w->hp->scanControlBtn(), this);
	sbc->enableScan(scanState);
	return scanState;
}

QStringList ScopyMainWindow_API::getDevicesName()
{
	Q_ASSERT(m_w->dm != nullptr);
	QStringList devicesName = m_w->dm->map.keys();
	return devicesName;
}

bool ScopyMainWindow_API::connectDevice(int idx)
{
	Device *dev = getDevice(idx);
	bool isConnected = false;
	bool successfulConnection = false;
	if(dev) {
		isConnected = m_w->dm->connectedDev.contains(dev->id());
		if(!isConnected) {
			dev->connectDev();
			successfulConnection = true;
		} else {
			qWarning(CAT_SCOPY_API) << "The device is already connected!";
		}
	} else {
		qWarning(CAT_SCOPY_API) << "The device is not available!";
	}
	return successfulConnection;
}

bool ScopyMainWindow_API::connectDevice(QString devID)
{
	Q_ASSERT(m_w->dm != nullptr);
	Device *dev = m_w->dm->getDevice(devID);
	bool successfulConnection = false;
	if(dev) {
		bool isConnected = m_w->dm->connectedDev.contains(dev->id());
		if(!isConnected) {
			dev->connectDev();
			successfulConnection = true;
		} else {
			qWarning(CAT_SCOPY_API) << "The device is already connected!";
		}
	} else {
		qWarning(CAT_SCOPY_API) << "The device is not available!";
	}
	return successfulConnection;
}

bool ScopyMainWindow_API::disconnectDevice(QString devID)
{
	Q_ASSERT(m_w->dm != nullptr);
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		dev->disconnectDev();
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
		return false;
	}
	return true;
}

bool ScopyMainWindow_API::disconnectDevice()
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		dev->disconnectDev();
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
		return false;
	}
	return true;
}

bool ScopyMainWindow_API::switchTool(QString devID, QString toolName)
{
	Q_ASSERT(m_w->dm != nullptr);
	bool toolSwitched = false;
	if(!m_w->dm->connectedDev.contains(devID)) {
		qWarning(CAT_SCOPY_API) << "Device " << devID << " not connected";
		return toolSwitched;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(dev->toolList(), toolName);
		if(!tool) {
			qWarning(CAT_SCOPY_API) << "Tool " << toolName << " doesn't exist for " << dev->displayName();
		} else {
			Q_EMIT m_w->dm->requestTool(tool->uuid());
			toolSwitched = true;
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return toolSwitched;
}

bool ScopyMainWindow_API::switchTool(QString toolName)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	bool toolSwitched = false;
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	} else {
		qWarning(CAT_SCOPY_API) << "No device connected";
		return toolSwitched;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(dev->toolList(), toolName);
		if(!tool) {
			qWarning(CAT_SCOPY_API) << "Tool " << toolName << " doesn't exist for " << dev->displayName();
		} else {
			Q_EMIT m_w->dm->requestTool(tool->uuid());
			toolSwitched = true;
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return toolSwitched;
}

void ScopyMainWindow_API::runScript(QString scriptPath, bool exitApp)
{
	QFile file(scriptPath);
	if(!file.open(QFile::ReadOnly)) {
		qCritical(CAT_SCOPY_API) << "Unable to open the script file: " << scriptPath;
		return;
	}
	const QString scriptContent = getScriptContent(&file);
	QJSValue val = ScopyJS::GetInstance()->engine()->evaluate(scriptContent, scriptPath);
	int ret = EXIT_SUCCESS;
	if(val.isError()) {
		qWarning(CAT_SCOPY_API) << "Exception:" << val.toString();
		ret = EXIT_FAILURE;
	} else if(!val.isUndefined()) {
		qWarning(CAT_SCOPY_API) << val.toString();
	}

	qInfo(CAT_SCOPY_API) << "Script finished with status" << ret;
	/* Exit application */
	if(exitApp)
		qApp->exit(ret);
}

void ScopyMainWindow_API::runScriptList(QStringList scriptPathList, bool exitApp)
{
	foreach(QString scriptPath, scriptPathList) {
		runScript(scriptPath, false);
	}

	if(exitApp) {
		int ret = EXIT_SUCCESS;
		qApp->exit(ret);
	}
}

const QString ScopyMainWindow_API::getScriptContent(QFile *file)
{
	QTextStream stream(file);
	QString firstLine = stream.readLine();
	if(!firstLine.startsWith("#!"))
		stream.seek(0);

	QString content = stream.readAll();
	file->close();
	return content;
}

bool ScopyMainWindow_API::sortByUUID(const QString &k1, const QString &k2)
{
	return k1.split("_").last() < k2.split("_").last();
}

void ScopyMainWindow_API::exit() { qApp->exit(); }

QStringList ScopyMainWindow_API::getTools()
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	QStringList resultList;
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	} else {
		qWarning(CAT_SCOPY_API) << "No device connected";
		return resultList;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		QList<ToolMenuEntry *> toolList = dev->toolList();
		for(int i = 0; i < toolList.size(); i++) {
			resultList.append(toolList[i]->name());
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return resultList;
}

QStringList ScopyMainWindow_API::getToolsForPlugin(QString plugin)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	QStringList resultList;
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	} else {
		qWarning(CAT_SCOPY_API) << "No device connected";
		return resultList;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		QList<ToolMenuEntry *> toolList = dev->toolList();
		for(int i = 0; i < toolList.size(); i++) {
			if(toolList[i]->pluginName() == plugin) {
				resultList.append(toolList[i]->name());
			}
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return resultList;
}

QPair<QString, QVariant> ScopyMainWindow_API::getPreference(QString prefName)
{
	Preferences *p = Preferences::GetInstance();
	QMap<QString, QVariant> prefMap = p->getPreferences();
	if(prefName != NULL) {
		return QPair<QString, QVariant>(prefName, prefMap[prefName]);
	}
}

QMap<QString, QVariant> ScopyMainWindow_API::getPreferences()
{
	Preferences *p = Preferences::GetInstance();
	QMap<QString, QVariant> prefMap = p->getPreferences();
	return prefMap;
}

void ScopyMainWindow_API::setPreference(QString prefName, QVariant value)
{
	Preferences *p = Preferences::GetInstance();
	p->set(prefName, value);
	if(prefName == "general_use_opengl") {
		qWarning(CAT_SCOPY_API) << "Restart is required for the change to take place";

	} else if(prefName == "general_theme") {
		qWarning(CAT_SCOPY_API) << "Restart is required for the change to take place";

	} else if(prefName == "general_language") {
		qWarning(CAT_SCOPY_API) << "Restart is required for the change to take place";
	}
}

void ScopyMainWindow_API::aboutPage()
{
	QString path = ":/about.html";
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning(CAT_SCOPY_API) << "Could not open the file:" << path;
	}

	QTextStream in(&file);

	while(!in.atEnd()) {
		QString line = in.readLine();
		qInfo(CAT_SCOPY_API) << line;
	}

	file.close();
}

QStringList ScopyMainWindow_API::getPlugins(int idx)
{
	Device *dev = getDevice(idx);
	QStringList pluginList;
	if(dev) {
		pluginList = availablePlugins("", "", dev);
	} else {
		qWarning(CAT_SCOPY_API) << "Device not found";
	}
	return pluginList;
}

QStringList ScopyMainWindow_API::getPlugins(QString uri, QString cat)
{
	QStringList pluginList;
	auto &&cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(uri);
	if(conn) {
		pluginList = availablePlugins(uri, cat, nullptr);
	} else {
		qWarning(CAT_SCOPY_API) << "Device not found";
	}
	return pluginList;
}

QStringList ScopyMainWindow_API::availablePlugins(QString uri, QString cat, Device *dev)
{
	PluginRepository *pr = new PluginRepository(this);
	m_w->loadPluginsFromRepository(pr);
	PluginManager *pm = pr->getPluginManager();
	QList<Plugin *> compatiblePlugins;
	QStringList resultList;
	if(!uri.isEmpty()) {
		compatiblePlugins = pm->getCompatiblePlugins(uri, cat);
	} else {
		compatiblePlugins = pm->getCompatiblePlugins(dev->param(), dev->category());
	}

	for(int i = 0; i < compatiblePlugins.size(); i++) {
		resultList.append(compatiblePlugins[i]->name());
	}
	return resultList;
}

bool ScopyMainWindow_API::getToolBtnState(QString tool)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	} else {
		qWarning(CAT_SCOPY_API) << "No device connected";
		return false;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		QList<ToolMenuEntry *> toolList = dev->toolList();
		for(int i = 0; i < toolList.size(); i++) {
			if(toolList[i]->name() == tool) {
				return toolList[i]->runBtnVisible();
			}
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return false;
}

bool ScopyMainWindow_API::runTool(QString tool, bool flag)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	bool toolStateChange = false;
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	} else {
		qWarning(CAT_SCOPY_API) << "No device connected";
		return toolStateChange;
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		QList<ToolMenuEntry *> toolList = dev->toolList();
		for(int i = 0; i < toolList.size(); i++) {
			if(toolList[i]->name() == tool) {
				toolList[i]->setRunning(flag);
				toolList[i]->setRunEnabled(flag);
				toolStateChange = true;
			}
		}
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
	return toolStateChange;
}

bool ScopyMainWindow_API::loadSetup(QString filename, QString path)
{
	QString fullPath = path + "/" + filename;
	bool setupLoaded = false;
	if(QFile::exists(fullPath)) {
		m_w->load(fullPath);
		setupLoaded = true;
	} else {
		qWarning(CAT_SCOPY_API) << "File " << filename << " does not exist";
	}
	return setupLoaded;
}

bool ScopyMainWindow_API::saveSetup(QString filename, QString path)
{
	Q_ASSERT(m_w->dm != nullptr);
	bool setupSaved = false;
	if(!m_w->dm->connectedDev.isEmpty()) {
		m_w->save(path + "/" + filename);
		qInfo(CAT_SCOPY_API) << "Setup saved at: " << path;
		setupSaved = true;
	} else {
		qWarning(CAT_SCOPY_API) << "Unable to save setup: No device connected";
	}
	return setupSaved;
}
#include "moc_scopymainwindow_api.cpp"
