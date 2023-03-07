#include "pluginrepository.h"
#include <QDir>
#include <QLibrary>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PLUGINREPOSTIORY,"PluginRepository");

using namespace adiscope;
PluginRepository::PluginRepository(QObject *parent) : QObject(parent)
{
	pm = new PluginManager(this);
}

PluginRepository::~PluginRepository()
{

}

void PluginRepository::init(QString location)
{
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
			qCritical(CAT_PLUGINREPOSTIORY) << QString(" ").repeated(err.offset)+"^";
		}
		else {
			qDebug(CAT_PLUGINREPOSTIORY) << "Found valid json @ "<<pluginMetaFilePath;
		}
		metadata = pluginMetaDocument.object();
		pm->setMetadata(metadata);
	}

	pm->clear();
	pm->add(pluginFiles);
	pm->sort();
}




#include "moc_pluginrepository.cpp"
