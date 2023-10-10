#ifndef SCOPYMAINWINDOW_API_H
#define SCOPYMAINWINDOW_API_H

#include "scopy-core_export.h"
#include "scopymainwindow.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScopyMainWindow_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ScopyMainWindow_API(ScopyMainWindow *w);
	~ScopyMainWindow_API();

	Q_INVOKABLE void acceptLicense();
	Q_INVOKABLE QString addDevice(QString cat, QString uri);
	Q_INVOKABLE bool connectDevice(int idx);
	Q_INVOKABLE bool connectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice(QString devID);
	Q_INVOKABLE bool disconnectDevice();
	Q_INVOKABLE void switchTool(QString devID, QString toolName);
	Q_INVOKABLE void switchTool(QString toolName);
	Q_INVOKABLE void runScript(QString content, QString fileName);

private:
	static bool sortByUUID(const QString &k1, const QString &k2);
	ScopyMainWindow *m_w;
};

} // namespace scopy

#endif // SCOPYMAINWINDOW_API_H
