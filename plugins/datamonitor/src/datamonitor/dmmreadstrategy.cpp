#include "dmmreadstrategy.hpp"
#include <QDebug>

using namespace scopy;
using namespace datamonitor;

DMMReadStrategy::DMMReadStrategy(iio_device *dev, iio_channel *chn)
	: dev(dev)
	, chn(chn)
{}

void DMMReadStrategy::setUmScale(double scale) { m_umScale = scale; }

void DMMReadStrategy::read()
{
	double raw = 0;
	int readRaw = iio_channel_attr_read_double(chn, "raw", &raw);

	double scale = 0;
	int readScale = iio_channel_attr_read_double(chn, "scale", &scale);

	double offset = 0;
	int readOffset = iio_channel_attr_read_double(chn, "offset", &offset);

	if(readRaw < 0) {
		char err[1024];
		iio_strerror(-(int)readRaw, err, sizeof(err));
		qDebug() << "device read error " << err;

	} else if(readScale < 0) {
		char err[1024];
		iio_strerror(-(int)readScale, err, sizeof(err));
		qDebug() << "device read error " << err;
	} else if(readOffset < 0) {
		char err[1024];
		iio_strerror(-(int)readOffset, err, sizeof(err));
		qDebug() << "device read error " << err;
	} else {
		double result = (raw + offset) * scale * m_umScale;
		qDebug() << "dmm read success  ";
		testDataTime += 1;
		Q_EMIT readDone(testDataTime, result);
	}
}
