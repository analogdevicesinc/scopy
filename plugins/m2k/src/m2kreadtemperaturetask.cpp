#include "m2kreadtemperaturetask.h"

#include "m2kcommon.h"

#include <QDebug>

#include <iioutil/connectionprovider.h>

using namespace scopy::m2k;

M2kReadTemperatureTask::M2kReadTemperatureTask(QString uri)
	: QThread()
{
	m_uri = uri;
}

void M2kReadTemperatureTask::run()
{
	double val, scale, offset, temperature;
	iio_device *dev;
	iio_channel *ch;
	int ret;

	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	if(!conn)
		goto finish;
	dev = iio_context_find_device(conn->context(), "ad9963");
	if(!dev)
		goto finish;
	ch = iio_device_find_channel(dev, "temp0", false);
	if(!ch)
		goto finish;

	ret = iio_channel_attr_read_double(ch, "raw", &val);
	if(ret)
		goto finish;
	iio_channel_attr_read_double(ch, "scale", &scale);
	if(ret)
		goto finish;
	iio_channel_attr_read_double(ch, "offset", &offset);
	if(ret)
		goto finish;

	temperature = (val + offset) * scale / 1000;
	qDebug(CAT_M2KPLUGIN) << "Read temperature" << temperature;

	Q_EMIT newTemperature(temperature);

finish:
	if(conn)
		ConnectionProvider::GetInstance()->close(m_uri);
	return;
}
