#ifndef GRIIOFLOATCHANNELSRC_H
#define GRIIOFLOATCHANNELSRC_H

#include "scopy-gr-util_export.h"
#include "griiodevicesource.h"
#include "gnuradio/blocks/short_to_float.h"

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRIIOFloatChannelSrc : public GRIIOChannel {
public:
	GRIIOFloatChannelSrc(GRIIODeviceSource* dev,QString channelName,QObject *parent = nullptr);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	gr::blocks::short_to_float::sptr s2f;

};
}
#endif // GRIIOFLOATCHANNELSRC_H
