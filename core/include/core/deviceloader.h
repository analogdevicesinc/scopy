#ifndef DEVICELOADER_H
#define DEVICELOADER_H

#include <QObject>
#include "deviceimpl.h"

namespace scopy {


class DeviceLoader : public QObject {
	Q_OBJECT
public:
	DeviceLoader(DeviceImpl *d, QObject *parent = nullptr);
	~DeviceLoader();
	void init();
Q_SIGNALS:
	void initialized();

private:
	DeviceImpl *d;
	QObject *oldParent;
	static constexpr bool threaded = true;

};
}

#endif // DEVICELOADER_H
