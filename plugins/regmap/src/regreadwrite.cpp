#include "regreadwrite.hpp"
#include <iio.h>
#include <qdebug.h>
#include <QLoggingCategory>
#include "logging_categories.h"

//Q_LOGGING_CATEGORY(CAT_IIO_OPERATION, "RegReadWrite")

RegReadWrite::RegReadWrite(struct iio_device *dev, QObject *parent)
	: dev(dev),
	  QObject{parent}
{}

RegReadWrite::~RegReadWrite()
{}

void RegReadWrite::read(uint32_t address)
{
	uint32_t reg_val;

	ssize_t read = iio_device_reg_read(dev, address, &reg_val);
	if (read < 0) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << read;
		Q_EMIT readError("device read error");
	} else {
		qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value " <<reg_val;
		Q_EMIT readDone(address,reg_val);
	}
}

void RegReadWrite::write(uint32_t address, uint32_t val)
{
	ssize_t write = iio_device_reg_write(dev, address, val);
	if (write < 0) {
		qDebug(CAT_IIO_OPERATION) << "device write error " << write;
		Q_EMIT writeError("device write err");
	} else {
		qDebug(CAT_IIO_OPERATION) << "device write successfull for register " << address << " with value " << val;
		Q_EMIT writeSuccess(address);
	}
}
