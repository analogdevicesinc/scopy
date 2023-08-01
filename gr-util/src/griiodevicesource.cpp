#include "griiodevicesource.h"
#include "griiofloatchannelsrc.h"
#include "griiocomplexchannelsrc.h"
#include "grtopblock.h"
#include "grlog.h"

using namespace scopy::grutil;
GRIIODeviceSource::GRIIODeviceSource(iio_context *ctx, QString deviceName, QString phyDeviceName, unsigned int buffersize, QObject *parent) :
	GRProxyBlock(parent), m_ctx(ctx), m_deviceName(deviceName), m_phyDeviceName(phyDeviceName), m_buffersize(buffersize)
{

}

void GRIIODeviceSource::addChannelAtIndex(iio_device* iio_dev, QString channelName) {
	std::string channel_name = channelName.toStdString();
	iio_channel* iio_ch = iio_device_find_channel(iio_dev, channel_name.c_str(), false);
	int idx = iio_channel_get_index(iio_ch);
	m_channelNames[idx] = channel_name;
}

void GRIIODeviceSource::computeChannelNames() {

	iio_device* iio_dev = iio_context_find_device(m_ctx,m_deviceName.toStdString().c_str());
	int max_channels = iio_device_get_channels_count(iio_dev);

	for(int i = 0;i<max_channels;i++) {
		m_channelNames.push_back(std::string());
	}

	for(GRIIOChannel* ch : qAsConst(m_list)) {
		GRIIOFloatChannelSrc* floatCh = dynamic_cast<GRIIOFloatChannelSrc*>(ch);
		if(floatCh) {
			addChannelAtIndex(iio_dev, floatCh->getChannelName());
		}

		GRIIOComplexChannelSrc* complexCh = dynamic_cast<GRIIOComplexChannelSrc*>(ch);
		if(complexCh) {
			addChannelAtIndex(iio_dev, complexCh->getChannelNameI());
			addChannelAtIndex(iio_dev, complexCh->getChannelNameQ());
		}
	}

	m_channelNames.erase(std::remove_if(
				 m_channelNames.begin(),
				 m_channelNames.end(),
				 [=](std::string x){return x.empty();}),
			     m_channelNames.end()); // clear empty channels

}

int GRIIODeviceSource::getOutputIndex(QString ch) {
	for(int i = 0;i < m_channelNames.size();i++) {
		if(ch.toStdString() == m_channelNames[i])
			return i;
	}
	return -1;
}

const iio_data_format* GRIIODeviceSource::getChannelFormat(QString ch) {
	std::string channel_name = ch.toStdString();
	iio_device* iio_dev = iio_context_find_device(m_ctx,m_deviceName.toStdString().c_str());
	iio_channel* iio_ch = iio_device_find_channel(iio_dev, channel_name.c_str(), false);
	return iio_channel_get_data_format(iio_ch);
}

void GRIIODeviceSource::matchChannelToBlockOutputs(GRTopBlock *top) {
	for(GRIIOChannel* ch : qAsConst(m_list)) {
		GRIIOFloatChannelSrc* floatCh = dynamic_cast<GRIIOFloatChannelSrc*>(ch);
		if(floatCh) {
			auto start_sptr = floatCh->getGrStartPoint();
			top->connect(src, getOutputIndex(floatCh->getChannelName()),start_sptr[0],0);
		}

		GRIIOComplexChannelSrc* complexCh = dynamic_cast<GRIIOComplexChannelSrc*>(ch);
		if(complexCh) {
			auto start_sptr = complexCh->getGrStartPoint();
			top->connect(src, getOutputIndex(complexCh->getChannelNameI()),start_sptr[0],0);
			top->connect(src, getOutputIndex(complexCh->getChannelNameQ()),start_sptr[1],0);
		}
	}
}

void GRIIODeviceSource::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL)<<"Building GRIIODeviceSource";
	if(m_list.count() == 0)
		return;

	computeChannelNames();
	// create block
	src = gr::iio::device_source::make_from(m_ctx, m_deviceName.toStdString(), m_channelNames, m_phyDeviceName.toStdString(), gr::iio::iio_param_vec_t(), m_buffersize);
	src->set_output_multiple(m_buffersize);
	src->set_len_tag_key("buffer_start");
	// match channels with blocks

	end_blk = src;

}

void GRIIODeviceSource::destroy_blks(GRTopBlock *top)
{
	m_list.clear();
	m_channelNames.clear();
	src = nullptr;
	end_blk = nullptr;
}

void GRIIODeviceSource::connect_blk(GRTopBlock *top, GRProxyBlock *)
{
	matchChannelToBlockOutputs(top);
}

void GRIIODeviceSource::disconnect_blk(GRTopBlock *top)
{
	top->getGrBlock()->disconnect(src);
}

void GRIIODeviceSource::addChannel(GRIIOChannel *ch)
{
	m_list.append(ch);
}

void GRIIODeviceSource::removeChannel(GRIIOChannel *ch)
{
	m_list.removeAll(ch);
}

unsigned int GRIIODeviceSource::getBuffersize() const
{
	return m_buffersize;
}

void GRIIODeviceSource::setBuffersize(unsigned int newBuffersize)
{
	m_buffersize = newBuffersize;
	Q_EMIT requestRebuild();
}

std::vector<std::string> GRIIODeviceSource::channelNames() const
{
	return m_channelNames;
}

QString GRIIODeviceSource::deviceName() const
{
	return m_deviceName;
}
