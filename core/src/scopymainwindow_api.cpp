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

QString ScopyMainWindow_API::addDevice(QString cat, QString uri)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	devID = m_w->dm->createDevice(cat, uri, false);
	qInfo(CAT_SCOPY_API) << "Device with id " << devID << " has been created!";
	return devID;
}

bool ScopyMainWindow_API::connectDevice(int idx)
{
	Q_ASSERT(m_w->dm != nullptr);
	Device *dev = nullptr;
	bool isConnected = false;
	bool successfulConnection = false;
	QList mapKeys = m_w->dm->map.keys();
	std::sort(mapKeys.begin(), mapKeys.end(), sortByUUID);
	if(idx < mapKeys.size()) {
		dev = m_w->dm->map[mapKeys[idx]];
	}
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

void ScopyMainWindow_API::switchTool(QString devID, QString toolName)
{
	Q_ASSERT(m_w->dm != nullptr);
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(dev->toolList(), toolName);
		if(!tool) {
			qWarning(CAT_SCOPY_API) << "Tool " << toolName << " doesn't exist for " << dev->displayName();
			return;
		}
		Q_EMIT m_w->dm->requestTool(tool->uuid());
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
}

void ScopyMainWindow_API::switchTool(QString toolName)
{
	Q_ASSERT(m_w->dm != nullptr);
	QString devID = "";
	if(!m_w->dm->connectedDev.isEmpty()) {
		devID = m_w->dm->connectedDev.back();
	}
	Device *dev = m_w->dm->getDevice(devID);
	if(dev) {
		ToolMenuEntry *tool = ToolMenuEntry::findToolMenuEntryByName(dev->toolList(), toolName);
		if(!tool) {
			qWarning(CAT_SCOPY_API) << "Tool " << toolName << " doesn't exist for " << dev->displayName();
			return;
		}
		Q_EMIT m_w->dm->requestTool(tool->uuid());
	} else {
		qWarning(CAT_SCOPY_API) << "Device with id " << devID << " is not available!";
	}
}

void ScopyMainWindow_API::runScript(QString content, QString fileName)
{
	QJSValue val = ScopyJS::GetInstance()->engine()->evaluate(content, fileName);
	int ret = EXIT_SUCCESS;
	if(val.isError()) {
		qWarning(CAT_SCOPY_API) << "Exception:" << val.toString();
		ret = EXIT_FAILURE;
	} else if(!val.isUndefined()) {
		qWarning(CAT_SCOPY_API) << val.toString();
	}

	/* Exit application */
	qApp->exit(ret);
}

bool ScopyMainWindow_API::sortByUUID(const QString &k1, const QString &k2)
{
	return k1.split("_").last() < k2.split("_").last();
}

#include "moc_scopymainwindow_api.cpp"
