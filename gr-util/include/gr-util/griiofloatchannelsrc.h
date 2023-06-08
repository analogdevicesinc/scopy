#ifndef GRIIOFLOATCHANNELSRC_H
#define GRIIOFLOATCHANNELSRC_H

#include "scopy-gr-util_export.h"
#include "griiodevicesource.h"

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRIIOFloatChannelSrc : public GRIIOChannel {
public:
	GRIIOFloatChannelSrc(GRIIODeviceSource* dev,QString channelName,QObject *parent = nullptr);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	gr::basic_block_sptr x2f;
};
}
#endif // GRIIOFLOATCHANNELSRC_H
