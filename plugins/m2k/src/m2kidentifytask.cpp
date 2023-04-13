#include "m2kidentifytask.h"
#include <iio.h>
#include "iioutil/contextprovider.h"

using namespace scopy::m2k;
M2kIdentifyTask::M2kIdentifyTask(QString uri, QObject *parent) : QThread(parent), m_uri(uri) {}

M2kIdentifyTask::~M2kIdentifyTask() {}

void M2kIdentifyTask::run() {
	iio_context *ctx = ContextProvider::GetInstance()->open(m_uri);
	iio_device *dev;
	iio_channel *ch;

	if(!ctx)
		return;
	dev = iio_context_find_device(ctx,"m2k-fabric");
	if(!dev)
		return;
	ch = iio_device_find_channel(dev,"voltage4",true);
	if(!ch)
		return;

	for( int i = 0;i<10;i++) {
		iio_channel_attr_write(ch,"done_led_overwrite_powerdown","1");
		QThread::msleep(100);
		iio_channel_attr_write(ch,"done_led_overwrite_powerdown","0");
		QThread::msleep(100);
		if(isInterruptionRequested())
			break;
	}

finish:
	ContextProvider::GetInstance()->close(m_uri);
	return;
}
