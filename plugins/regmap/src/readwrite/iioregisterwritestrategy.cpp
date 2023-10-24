#include "iioregisterwritestrategy.hpp"

#include "../logging_categories.h"

using namespace scopy::regmap;

IIORegisterWriteStrategy::IIORegisterWriteStrategy(struct iio_device *dev)
	: dev(dev)
{}

void IIORegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	address = address | offset;
	ssize_t write = iio_device_reg_write(dev, address, val);
	if(write < 0) {
		char err[1024];
		iio_strerror(-(int)write, err, sizeof(err));
		qDebug(CAT_IIO_OPERATION) << "device write error " << err;
		Q_EMIT writeError("device write err");
	} else {
		qDebug(CAT_IIO_OPERATION)
			<< "device write successfull for register " << address << " with value " << val;
		Q_EMIT writeSuccess(address);
	}
}

uint32_t IIORegisterWriteStrategy::getOffset() const { return offset; }

void IIORegisterWriteStrategy::setOffset(uint32_t newOffset) { offset = newOffset; }
