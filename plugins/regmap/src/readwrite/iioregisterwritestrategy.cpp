#include "iioregisterwritestrategy.hpp"

#include "../logging_categories.h"

using namespace scopy::regmap;

IIORegisterWriteStrategy::IIORegisterWriteStrategy(struct iio_device *dev)
	: dev(dev)
{}

void IIORegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	uint32_t auxAddress = address | addressSpace;
	ssize_t write = iio_device_reg_write(dev, auxAddress, val);
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

uint32_t IIORegisterWriteStrategy::getAddressSpace() const { return addressSpace; }

void IIORegisterWriteStrategy::setAddressSpace(uint32_t newAddressSpace) { addressSpace = newAddressSpace; }
