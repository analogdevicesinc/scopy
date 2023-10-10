#ifndef DEVICELOADER_H
#define DEVICELOADER_H

#include "deviceimpl.h"

#include <QObject>

namespace scopy {

class DeviceLoader : public QObject
{
	Q_OBJECT
public:
	DeviceLoader(DeviceImpl *d, QObject *parent = nullptr);
	~DeviceLoader();
	void init(bool async = true);
	void asyncInit();
	void syncInit();
Q_SIGNALS:
	void initialized();

private:
	DeviceImpl *d;
	QObject *oldParent;
};
} // namespace scopy

#endif // DEVICELOADER_H
