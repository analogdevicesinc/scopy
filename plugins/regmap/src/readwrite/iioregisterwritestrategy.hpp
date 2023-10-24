#ifndef IIOREGISTERWRITESTRATEGY_HPP
#define IIOREGISTERWRITESTRATEGY_HPP

#include "iregisterwritestrategy.hpp"

#include <iio.h>

namespace scopy::regmap {
class IIORegisterWriteStrategy : public IRegisterWriteStrategy
{
public:
	IIORegisterWriteStrategy(struct iio_device *dev);

private:
	struct iio_device *dev;
	uint32_t offset = 0;

	// IRegisterWriteStrategy interface
public:
	void write(uint32_t address, uint32_t val);
	uint32_t getOffset() const;
	void setOffset(uint32_t newOffset);
};
} // namespace scopy::regmap
#endif // IIOREGISTERWRITESTRATEGY_HPP
