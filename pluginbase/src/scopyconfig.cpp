#include "scopyconfig.h"
#include "scopy-pluginbase_config.h"
#include <QStandardPaths>
#include <QApplication>
#include <QSysInfo>

QString scopy::config::defaultPluginFolderPath()
{
	return "./plugins/plugins";
}

QString scopy::config::preferencesFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::settingsFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::executableFolderPath()
{
	return QCoreApplication::applicationDirPath();
}

QString scopy::config::version()
{
	return SCOPY_VERSION;
}

QString scopy::config::gitCommit()
{
	return SCOPY_VERSION_GIT;
}

QString scopy::config::fullversion()
{
	QString ver = QString("v") + SCOPY_VERSION + "-g" + SCOPY_VERSION_GIT;
	if(QString(SCOPY_GIT_DIRTY) == "dirty") {
		ver += QString("-") + "dirty";
	}
	return ver;
}

QString scopy::config::os()
{
	return QSysInfo::prettyProductName();
}

QString scopy::config::pcSpecs()
{
	QString ret;
	ret.append("build_abi: "+QSysInfo::buildAbi());ret.append("\n");
	ret.append("build_cpu: "+QSysInfo::buildCpuArchitecture());ret.append("\n");
	ret.append("host: "+QSysInfo::machineHostName());ret.append("\n");
	ret.append("arch: "+QSysInfo::currentCpuArchitecture());ret.append("\n");
	ret.append("kernel: "+ QSysInfo::kernelType());ret.append("\n");
	ret.append("kernel-ver: "+ QSysInfo::kernelVersion());ret.append("\n");

	return ret;
}

QString scopy::config::dump() {
	QString ret;

	ret = QString("ScopyConfig");
	ret+= QString("\n") + "DefaultPluginFolderPath: " + defaultPluginFolderPath() ;
	ret+= QString("\n") + "PreferencesPluginFolderPath: " + preferencesFolderPath() ;
	ret+= QString("\n") + "SettingsFolderPath: " + settingsFolderPath() ;
	ret+= QString("\n") + "ExecutableFolderPath: " + executableFolderPath() ;
	ret+= QString("\n") + "version: " + version() ;
	ret+= QString("\n") + "gitCommit: " + gitCommit();
	ret+= QString("\n") + "fullversion: " + fullversion();
	ret+= QString("\n") + "os: " + os();
	ret+= QString("\n") + "pcSpecs: \n" + pcSpecs();

	return ret;
}
