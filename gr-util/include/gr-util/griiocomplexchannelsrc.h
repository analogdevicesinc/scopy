#ifndef GRIIOCOMPLEXCHANNELSRC_H
#define GRIIOCOMPLEXCHANNELSRC_H

#include "griiodevicesource.h"
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/stream_to_vector.h>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT GRIIOComplexChannelSrc : public GRIIOChannel
{
public:
	GRIIOComplexChannelSrc(QString channelName, GRIIODeviceSource *dev, QString channelNameI, QString channelNameQ,
			       QObject *parent = nullptr);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

	const QString &getChannelNameI() const;
	const QString &getChannelNameQ() const;

	const iio_data_format *getFmt() const;

protected:
	QString channelNameI;
	QString channelNameQ;

	const iio_data_format *fmt;
	gr::blocks::short_to_float::sptr s2f[2];
	gr::blocks::float_to_complex::sptr f2c;
	gr::blocks::stream_to_vector::sptr s2v;
};
} // namespace scopy::grutil
#endif // GRIIOCOMPLEXCHANNELSRC_H
