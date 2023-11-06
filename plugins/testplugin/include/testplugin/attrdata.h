#ifndef SCOPY_ATTRDATA_H
#define SCOPY_ATTRDATA_H

#include <iio.h>
#include <QString>

struct AttributeFactoryRecipe
{
	struct iio_context *context = nullptr; // the context in which to search for iio structures
	struct iio_device *device = nullptr;   // the device in which to search for iio structures
	struct iio_channel *channel = nullptr; // the channel in which to search for iio structures
	QString data = "";	  // the name of the 'main' attribute that will be changed in the iio channel
	QString dataOptions = ""; // the attribute that describes what values can the attribute take
};

#endif // SCOPY_ATTRDATA_H
