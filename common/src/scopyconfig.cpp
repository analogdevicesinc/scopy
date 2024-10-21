#include "scopyconfig.h"

#include "scopy-common_config.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSysInfo>
#include <QUuid>

QString scopy::config::tempLogFilePath() { return QDir::cleanPath(settingsFolderPath() + "/" + SCOPY_TEMP_LOG_FILE); }

QString scopy::config::defaultPluginFolderPath()
{

#ifdef WIN32
	// Scopy_install_folder/plugins
	return QCoreApplication::applicationDirPath() + "/plugins";
#elif defined __APPLE__
	// Scopy.app/Contents/MacOS/plugins
	return QCoreApplication::applicationDirPath() + "/plugins";
#elif defined(__appimage__)
	// usr/lib/plugins
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/plugins";
#endif

	return SCOPY_PLUGIN_INSTALL_PATH;
}

QString scopy::config::localPluginFolderPath() { return SCOPY_PLUGIN_BUILD_PATH; }

QString scopy::config::defaultTranslationFolderPath()
{
#if defined __APPLE__
	return QCoreApplication::applicationDirPath() + "/translations";
#elif defined(__appimage__)
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/translations";
#endif
	return SCOPY_TRANSLATION_INSTALL_PATH;
}

QString scopy::config::localTranslationFolderPath() { return SCOPY_TRANSLATION_BUILD_PATH; }

QString scopy::config::defaultStyleFolderPath()
{
#if defined __APPLE__
	return QCoreApplication::applicationDirPath() + "/style";
#elif defined(__appimage__)
	return QCoreApplication::applicationDirPath() + "/../lib/scopy/style";
#endif
	return SCOPY_STYLE_INSTALL_PATH;
}

QString scopy::config::localStyleFolderPath() { return SCOPY_STYLE_BUILD_PATH; }

QString scopy::config::preferencesFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::settingsFolderPath()
{
	return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString scopy::config::executableFolderPath() { return QCoreApplication::applicationDirPath(); }

QString scopy::config::version() { return SCOPY_VERSION; }

QString scopy::config::gitCommit() { return SCOPY_VERSION_GIT; }

QString scopy::config::fullversion()
{
	QString ver = QString("v") + SCOPY_VERSION + "-g" + SCOPY_VERSION_GIT;
	if(QString(SCOPY_GIT_DIRTY) == "dirty") {
		ver += QString("-") + "dirty";
	}
	return ver;
}

QString scopy::config::os() { return QSysInfo::prettyProductName(); }

QString scopy::config::pcSpecs()
{
	QString ret;
	ret.append("build_abi: " + QSysInfo::buildAbi());
	ret.append("\n");
	ret.append("build_cpu: " + QSysInfo::buildCpuArchitecture());
	ret.append("\n");
	ret.append("host: " + QSysInfo::machineHostName());
	ret.append("\n");
	ret.append("arch: " + QSysInfo::currentCpuArchitecture());
	ret.append("\n");
	ret.append("kernel: " + QSysInfo::kernelType());
	ret.append("\n");
	ret.append("kernel-ver: " + QSysInfo::kernelVersion());
	ret.append("\n");

	return ret;
}

QString scopy::config::dump()
{
	QString ret;

	ret = QString("ScopyConfig");
	ret += QString("\n") + "DefaultPluginFolderPath: " + defaultPluginFolderPath();
	ret += QString("\n") + "PreferencesPluginFolderPath: " + preferencesFolderPath();
	ret += QString("\n") + "SettingsFolderPath: " + settingsFolderPath();
	ret += QString("\n") + "ExecutableFolderPath: " + executableFolderPath();
	ret += QString("\n") + "version: " + version();
	ret += QString("\n") + "gitCommit: " + gitCommit();
	ret += QString("\n") + "fullversion: " + fullversion();
	ret += QString("\n") + "os: " + os();
	ret += QString("\n") + "pcSpecs: \n" + pcSpecs();

	return ret;
}

QString scopy::config::getUuid()
{
#ifdef USE_QUUID
	return QUuid.createUuid().toString();
#else
	return QString::number(uuid++);
#endif
}
