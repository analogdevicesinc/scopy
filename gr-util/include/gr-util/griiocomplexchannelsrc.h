#ifndef GRIIOCOMPLEXCHANNELSRC_H
#define GRIIOCOMPLEXCHANNELSRC_H

#include "griiodevicesource.h"
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/short_to_float.h>

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

protected:
	QString channelNameI;
	QString channelNameQ;
	gr::blocks::short_to_float::sptr s2f[2];
	gr::blocks::float_to_complex::sptr f2c;
};
} // namespace scopy::grutil
#endif // GRIIOCOMPLEXCHANNELSRC_H
