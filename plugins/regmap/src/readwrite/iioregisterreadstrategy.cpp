#include "iioregisterreadstrategy.hpp"

#include "../logging_categories.h"
#include "iregisterreadstrategy.hpp"

using namespace scopy;
using namespace regmap;

IIORegisterReadStrategy::IIORegisterReadStrategy(struct iio_device *dev)
	: dev(dev)
{}

void IIORegisterReadStrategy::read(uint32_t address)
{
	address = address | offset;
	uint32_t reg_val;

	ssize_t read = iio_device_reg_read(dev, address, &reg_val);
	if(read < 0) {
		char err[1024];
		iio_strerror(-(int)read, err, sizeof(err));
		qDebug(CAT_IIO_OPERATION) << "device read error " << err;
		Q_EMIT readError("device read error");
	} else {
		qDebug(CAT_IIO_OPERATION)
			<< "device read success for register " << address << " with value " << reg_val;
		Q_EMIT readDone(address, reg_val);
	}
}

uint32_t IIORegisterReadStrategy::getOffset() const { return offset; }

void IIORegisterReadStrategy::setOffset(uint32_t newOffset) { offset = newOffset; }
