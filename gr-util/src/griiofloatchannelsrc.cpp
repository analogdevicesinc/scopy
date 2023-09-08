#include "griiofloatchannelsrc.h"
#include "grlog.h"
#include "gnuradio/blocks/int_to_float.h"
#include "gnuradio/blocks/short_to_float.h"
#include "gnuradio/blocks/copy.h"

#include "gnuradio/blocks/char_to_float.h"


using namespace scopy::grutil;
GRIIOFloatChannelSrc::GRIIOFloatChannelSrc(GRIIODeviceSource *dev, QString channelName, QObject *parent) :
	GRIIOChannel(channelName, dev, parent)
{
	fmt = dev->getChannelFormat(channelName);
}

void GRIIOFloatChannelSrc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL)<<"Building GRIIOFloatChannelSrc";
	dev->addChannel(this);
	switch(fmt->length) {
	case 16:
		x2f = gr::blocks::short_to_float::make();
		break;
	case 32:
		x2f = gr::blocks::int_to_float::make();
		break;
	default:		
		qInfo(SCOPY_GR_UTIL)<<"creating copy block of size " << fmt->length/8;
		x2f = gr::blocks::copy::make(fmt->length/8);
		break;
	}
	end_blk = x2f;
	start_blk.append(x2f);
}

void GRIIOFloatChannelSrc::destroy_blks(GRTopBlock *top)
{
	dev->removeChannel(this);
	x2f = nullptr;
	end_blk = nullptr;
	start_blk.clear();
}

const iio_data_format *GRIIOFloatChannelSrc::getFmt() const
{
	return fmt;
}
