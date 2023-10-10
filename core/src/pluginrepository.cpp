#include "pluginrepository.h"

#ifdef Q_OS_WINDOWS
#include <windows.h>
#include <Winbase.h>
#endif

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLibrary>
#include <QLoggingCategory>
#include <QtGlobal>

Q_LOGGING_CATEGORY(CAT_PLUGINREPOSTIORY, "PluginRepository");

using namespace scopy;
PluginRepository::PluginRepository(QObject *parent)
	: QObject(parent)
{
	pm = new PluginManager(this);
}

PluginRepository::~PluginRepository() {}

void PluginRepository::init(QString location)
{
	qInfo(CAT_PLUGINREPOSTIORY) << "initializing plugins from: " << location;
	const QString pluginMetaFileName = "plugin.json";
	QString pluginMetaFilePath = "";
	QDir loc(location);

	QFileInfoList plugins = loc.entryInfoList(QDir::Files);
	QStringList pluginFiles;

	for(const QFileInfo &p : plugins) {
		if(p.fileName() == pluginMetaFileName) {
			pluginMetaFilePath = p.absoluteFilePath();
			continue;
		}
		pluginFiles.append(p.absoluteFilePath());
	}

	if(!pluginMetaFilePath.isEmpty()) {
		QFile f(pluginMetaFilePath);
		f.open(QFile::ReadOnly);
		QString json = f.readAll();
		QJsonParseError err;
		QJsonDocument pluginMetaDocument = QJsonDocument::fromJson(json.toUtf8(), &err);
		if(err.error != QJsonParseError::NoError) {
			qCritical(CAT_PLUGINREPOSTIORY) << "JSON Parse error !" << err.errorString();
			qCritical(CAT_PLUGINREPOSTIORY) << json;
			qCritical(CAT_PLUGINREPOSTIORY) << QString(" ").repeated(err.offset) + "^";
		} else {
			qDebug(CAT_PLUGINREPOSTIORY) << "Found valid json @ " << pluginMetaFilePath;
		}
		metadata = pluginMetaDocument.object();
		pm->setMetadata(metadata);
	}

#ifdef Q_OS_WINDOWS
	bool b = SetDllDirectoryA(QApplication::applicationDirPath().toStdString().c_str());
	if(!b) {
		DWORD error = ::GetLastError();
		std::string message = std::system_category().message(error);
		qWarning(CAT_PLUGINREPOSTIORY)
			<< "cannot add .exe folder to library search path - " << QString::fromStdString(message);
		;
	}
#endif

	pm->clear();
	pm->add(pluginFiles);
	pm->sort();
}

#include "moc_pluginrepository.cpp"
