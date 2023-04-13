#include "deviceloader.h"
#include <QThread>

using namespace scopy;

DeviceLoader::DeviceLoader(DeviceImpl* d, QObject *parent) : d(d), QObject(parent)
{

}

DeviceLoader::~DeviceLoader()
{

}

void DeviceLoader::init() {
	if(threaded) {
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
	} else {
		d->init();
		Q_EMIT initialized();
	}
}
