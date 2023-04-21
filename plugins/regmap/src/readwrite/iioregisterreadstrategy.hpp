#ifndef IIOREGISTERREADSTRATEGY_HPP
#define IIOREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

#include <QObject>

class IIORegisterReadStrategy : public IRegisterReadStrategy
{
public:
	explicit IIORegisterReadStrategy(struct iio_device *dev);
	void read(uint32_t address);
Q_SIGNALS:

private:
	struct iio_device *dev;
};

#endif // IIOREGISTERREADSTRATEGY_HPP
