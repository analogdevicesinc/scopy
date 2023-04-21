#ifndef IIOREGISTERWRITESTRATEGY_HPP
#define IIOREGISTERWRITESTRATEGY_HPP

#include "iregisterwritestrategy.hpp"

class IIORegisterWriteStrategy: public IRegisterWriteStrategy
{
public:
    IIORegisterWriteStrategy(struct iio_device *dev);

private:
    struct iio_device *dev;

    // IRegisterWriteStrategy interface
public:
    void write(uint32_t address, uint32_t val);
};

#endif // IIOREGISTERWRITESTRATEGY_HPP
