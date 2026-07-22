#include "v0backend.h"

extern "C" scopy::iio::IBackend *createIIOBackend()
{
    return new scopy::iio::V0Backend();
}
