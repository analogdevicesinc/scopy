#include "iioregisterreadstrategy.hpp"
#include <iio.h>
#include "iregisterreadstrategy.hpp"
#include "../logging_categories.h"


IIORegisterReadStrategy::IIORegisterReadStrategy(struct iio_device *dev)
	:dev(dev)
{
}

void IIORegisterReadStrategy::read(uint32_t address)
{
	uint32_t reg_val;

	ssize_t read = iio_device_reg_read(dev, address, &reg_val);
	if (read < 0) {
        char err[1024];
        iio_strerror(-(int)read, err, sizeof(err));
        qDebug(CAT_IIO_OPERATION) << "device read error " << err;
        Q_EMIT readError("device read error");
	} else {
        qDebug(CAT_IIO_OPERATION) << "device read success for register " << address << " with value " <<reg_val;
        Q_EMIT readDone(address,reg_val);
	}
}
