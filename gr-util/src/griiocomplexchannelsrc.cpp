#include "griiocomplexchannelsrc.h"
#include "griiodevicesource.h"
#include "grtopblock.h"
#include <QDebug>
#include "grlog.h"

using namespace scopy::grutil;
GRIIOComplexChannelSrc::GRIIOComplexChannelSrc(GRIIODeviceSource *dev, QString channelNameI, QString channelNameQ, QObject *parent) :
      GRIIOChannel(parent), dev(dev), channelNameI(channelNameI), channelNameQ(channelNameQ)
{
}

void GRIIOComplexChannelSrc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL)<<"Building GRIIOComplexChannelSrc";
	dev->addChannel(this);
	s2f[0] = gr::blocks::short_to_float::make();
	s2f[1] = gr::blocks::short_to_float::make();
	f2c = gr::blocks::float_to_complex::make();

	top->connect(s2f[0],0,f2c,0);
	top->connect(s2f[1],0,f2c,1);
	start_blk.append(s2f[0]);
	start_blk.append(s2f[1]);
	end_blk = f2c;
}

void GRIIOComplexChannelSrc::destroy_blks(GRTopBlock *top)
{
	dev->removeChannel(this);
	end_blk = nullptr;
	s2f[0] = s2f[1] = nullptr;
	f2c = nullptr;
	start_blk.clear();
}

const QString &GRIIOComplexChannelSrc::getChannelNameI() const
{
	return channelNameI;
}

const QString &GRIIOComplexChannelSrc::getChannelNameQ() const
{
	return channelNameQ;
}
