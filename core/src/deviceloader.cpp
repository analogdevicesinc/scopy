#include "deviceloader.h"
#include <QThread>

using namespace scopy;

DeviceLoader::DeviceLoader(DeviceImpl* d, QObject *parent) : d(d), QObject(parent)
{

}

DeviceLoader::~DeviceLoader()
{

}

void DeviceLoader::init(bool async) {
	if(async) {
		asyncInit();
	} else {
		syncInit();
	}
}

void DeviceLoader::asyncInit()
{
	QThread *th = QThread::create([=]{
		// initializer thread
		d->init();
	});
	oldParent = d->parent();
	d->setParent(nullptr);
	d->moveToThread(th);

	connect(th,&QThread::destroyed, this,[=]() {;
			// back to main thread
			d->moveToThread(QThread::currentThread());
			d->setParent(oldParent);
			Q_EMIT initialized();
		}, Qt::QueuedConnection);
	connect(th,&QThread::finished, th, &QThread::deleteLater);

	th->start();
}

void DeviceLoader::syncInit() {
	d->init();
	Q_EMIT initialized();
}

#include "moc_deviceloader.cpp"
