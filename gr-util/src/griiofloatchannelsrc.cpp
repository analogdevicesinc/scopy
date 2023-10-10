#include "griiofloatchannelsrc.h"

#include "gnuradio/blocks/char_to_float.h"
#include "gnuradio/blocks/copy.h"
#include "gnuradio/blocks/int_to_float.h"
#include "gnuradio/blocks/short_to_float.h"
#include "grlog.h"

using namespace scopy::grutil;
GRIIOFloatChannelSrc::GRIIOFloatChannelSrc(GRIIODeviceSource *dev, QString channelName, QObject *parent)
	: GRIIOChannel(channelName, dev, parent)
{
	m_iioCh = iio_device_find_channel(dev->iioDev(), channelName.toStdString().c_str(), false);
	fmt = iio_channel_get_data_format(m_iioCh);

	m_sampleRateAttribute = findAttribute(
		{
			"sample_rate",
			"sampling_rate",
			"sample_frequency",
			"sampling_frequency",
		},
		m_iioCh);
}

void GRIIOFloatChannelSrc::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL) << "Building GRIIOFloatChannelSrc";
	dev->addChannel(this);
	switch(fmt->length) {
	case 16:
		x2f = gr::blocks::short_to_float::make();
		break;
	case 32:
		x2f = gr::blocks::int_to_float::make();
		break;
	default:
		qInfo(SCOPY_GR_UTIL) << "creating copy block of size " << fmt->length / 8;
		x2f = gr::blocks::copy::make(fmt->length / 8);
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

bool GRIIOFloatChannelSrc::sampleRateAvailable()
{
	if(m_sampleRateAttribute.isEmpty())
		return false;
	return true;
}

double GRIIOFloatChannelSrc::readSampleRate()
{
	char buffer[20];
	bool ok = false;
	double sr;
	if(!sampleRateAvailable())
		return -1;

	iio_channel_attr_read(m_iioCh, m_sampleRateAttribute.toStdString().c_str(), buffer, 20);
	QString str(buffer);
	sr = str.toDouble(&ok);
	if(ok) {
		return sr;
	} else {
		return -1;
	}
}

const iio_data_format *GRIIOFloatChannelSrc::getFmt() const { return fmt; }
