#ifndef SCOPYMAINWINDOW_API_H
#define SCOPYMAINWINDOW_API_H

#include "scopy-core_export.h"
#include "scopymainwindow.h"
#include "scanbuttoncontroller.h"
#include "iioutil/connectionprovider.h"
#include <QFile>

namespace scopy {
class SCOPY_CORE_EXPORT ScopyMainWindow_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ScopyMainWindow_API(ScopyMainWindow *w);
	~ScopyMainWindow_API();

	Q_INVOKABLE void acceptLicense();
	Q_INVOKABLE QString addDevice(QString uri, QString cat = "iio", bool async = false);
	Q_INVOKABLE QString addDevice(QString uri, QList<QString> plugins, QString cat = "iio", bool async = false);
	Q_INVOKABLE bool removeDevice(QString uri, QString cat = "iio");
	Q_INVOKABLE bool removeDevice(int idx);
	Q_INVOKABLE bool startScan(bool scanState);
	Q_INVOKABLE QStringList getDevicesName();
	Q_INVOKABLE bool connectDevice(int idx);
	Q_INVOKABLE bool connectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice();
	Q_INVOKABLE bool switchTool(QString devID, QString toolName);
	Q_INVOKABLE bool switchTool(QString toolName);
	Q_INVOKABLE void runScript(QString scriptPath, bool exitApp = true);
	Q_INVOKABLE void runScriptList(QStringList scriptPathList, bool exitApp = true);
	Q_INVOKABLE void exit();
	Q_INVOKABLE QStringList getTools();
	Q_INVOKABLE QStringList getToolsForPlugin(QString plugin);
	Q_INVOKABLE QPair<QString, QVariant> getPreference(QString prfName);
	Q_INVOKABLE QMap<QString, QVariant> getPreferences();
	Q_INVOKABLE void setPreference(QString preName, QVariant value);
	Q_INVOKABLE void aboutPage();
	Q_INVOKABLE QStringList getPlugins(int idx);
	Q_INVOKABLE QStringList getPlugins(QString param, QString cat = "iio");
	Q_INVOKABLE bool getToolBtnState(QString tool);
	Q_INVOKABLE bool runTool(QString tool, bool flag);
	Q_INVOKABLE bool loadSetup(QString fileName, QString path = QCoreApplication::applicationDirPath());
	Q_INVOKABLE bool saveSetup(QString fileName, QString path = QCoreApplication::applicationDirPath());

private:
	static bool sortByUUID(const QString &k1, const QString &k2);
	const QString getScriptContent(QFile *file);
	QStringList availablePlugins(QString param, QString cat, Device *dev);
	Device *getDevice(int idx);
	ScopyMainWindow *m_w;
};

} // namespace scopy

#endif // SCOPYMAINWINDOW_API_H
