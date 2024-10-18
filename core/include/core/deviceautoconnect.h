#ifndef DEVICEAUTOCONNECT_H
#define DEVICEAUTOCONNECT_H

#include <QObject>

namespace scopy {
class DeviceAutoConnect
{
public:
	static void initPreferences();
	static void addDevice(QString uri, QStringList plugins);
	static void removeDevice(QString uri);
	static void clear();
	static bool isAutoConnectEnabled(QString uri);
};
} // namespace scopy
#endif // DEVICEAUTOCONNECT_H
