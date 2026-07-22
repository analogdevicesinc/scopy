#include "v1backend.h"

extern "C" scopy::iio::IBackend *createIIOBackend()
{
    return new scopy::iio::V1Backend();
}
