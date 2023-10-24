#ifndef IIOREGISTERREADSTRATEGY_HPP
#define IIOREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

#include <iio.h>

#include <QObject>

namespace scopy::regmap {

class IIORegisterReadStrategy : public IRegisterReadStrategy
{
public:
	explicit IIORegisterReadStrategy(struct iio_device *dev);
	void read(uint32_t address);
	uint32_t getOffset() const;
	void setOffset(uint32_t newOffset);

Q_SIGNALS:

private:
	struct iio_device *dev;
	uint32_t offset = 0;
};
} // namespace scopy::regmap
#endif // IIOREGISTERREADSTRATEGY_HPP
