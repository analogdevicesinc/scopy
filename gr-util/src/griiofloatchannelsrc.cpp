#include "griiofloatchannelsrc.h"
#include "grlog.h"

using namespace scopy::grutil;
GRIIOFloatChannelSrc::GRIIOFloatChannelSrc(GRIIODeviceSource *dev, QString channelName, QObject *parent) :
	  GRIIOChannel(channelName, dev, parent)
{
}

void GRIIOFloatChannelSrc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL)<<"Building GRIIOFloatChannelSrc";
	dev->addChannel(this);
	s2f = gr::blocks::short_to_float::make();
	end_blk = s2f;
	start_blk.append(s2f);
}

void GRIIOFloatChannelSrc::destroy_blks(GRTopBlock *top)
{
	dev->removeChannel(this);
	s2f = nullptr;
	end_blk = nullptr;
	start_blk.clear();
}
