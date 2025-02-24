/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "griiodevicesource.h"

#include "griiocomplexchannelsrc.h"
#include "griiofloatchannelsrc.h"
#include "grlog.h"
#include "grtopblock.h"

using namespace scopy::grutil;
QString GRIIODeviceSource::findAttribute(QStringList possibleNames, iio_device *dev)
{

	const char *attr = nullptr;
	for(const QString &name : possibleNames) {
		attr = iio_device_find_attr(dev, name.toStdString().c_str());
		if(attr)
			break;
	}
	QString attributeName = QString(attr);
	return attributeName;
}

iio_device *GRIIODeviceSource::iioDev() const { return m_iioDev; }

iio_context *GRIIODeviceSource::ctx() const { return m_ctx; }

QString GRIIOChannel::findAttribute(QStringList possibleNames, iio_channel *ch)
{
	const char *attr = nullptr;
	for(const QString &name : possibleNames) {
		attr = iio_channel_find_attr(ch, name.toStdString().c_str());
		if(attr)
			break;
	}
	QString attributeName = QString(attr);
	return attributeName;
}

GRIIODeviceSource::GRIIODeviceSource(iio_context *ctx, QString deviceName, QString phyDeviceName,
				     unsigned int buffersize, QObject *parent)
	: GRProxyBlock(parent)
	, m_ctx(ctx)
	, m_deviceName(deviceName)
	, m_phyDeviceName(phyDeviceName)
	, m_buffersize(buffersize)
{

	m_iioDev = iio_context_find_device(m_ctx, m_deviceName.toStdString().c_str());
	m_sampleRateAttribute = findAttribute(
		{
			"sample_rate",
			"sampling_rate",
			"sample_frequency",
			"sampling_frequency",
		},
		m_iioDev);
}

void GRIIODeviceSource::addChannelAtIndex(iio_device *iio_dev, QString channelName)
{
	std::string channel_name = channelName.toStdString();
	iio_channel *iio_ch = iio_device_find_channel(iio_dev, channel_name.c_str(), false);
	int idx = iio_channel_get_index(iio_ch);

	while(idx < m_channelNames.size() && m_channelNames[idx] != "" &&
	      QString::fromStdString(m_channelNames[idx]) != channelName) {
		idx++;
	}
	m_channelNames[idx] = channel_name;
}

void GRIIODeviceSource::computeChannelNames()
{

	int max_channels = iio_device_get_channels_count(m_iioDev);

	for(int i = 0; i < max_channels; i++) {
		m_channelNames.push_back(std::string());
	}

	for(GRIIOChannel *ch : qAsConst(m_list)) {
		GRIIOFloatChannelSrc *floatCh = dynamic_cast<GRIIOFloatChannelSrc *>(ch);
		if(floatCh) {
			addChannelAtIndex(m_iioDev, floatCh->getChannelName());
		}

		GRIIOComplexChannelSrc *complexCh = dynamic_cast<GRIIOComplexChannelSrc *>(ch);
		if(complexCh) {
			addChannelAtIndex(m_iioDev, complexCh->getChannelNameI());
			addChannelAtIndex(m_iioDev, complexCh->getChannelNameQ());
		}
	}

	m_channelNames.erase(
		std::remove_if(m_channelNames.begin(), m_channelNames.end(), [=](std::string x) { return x.empty(); }),
		m_channelNames.end()); // clear empty channels
}

int GRIIODeviceSource::getOutputIndex(QString ch)
{
	for(int i = 0; i < m_channelNames.size(); i++) {
		if(ch.toStdString() == m_channelNames[i])
			return i;
	}
	return -1;
}

/*const iio_data_format* GRIIODeviceSource::getChannelFormat(QString ch) {
	std::string channel_name = ch.toStdString();
	iio_channel* iio_ch = iio_device_find_channel(iio_dev, channel_name.c_str(), false);
	return iio_channel_get_data_format(iio_ch);
}*/

bool GRIIODeviceSource::sampleRateAvailable()
{
	if(m_sampleRateAttribute.isEmpty())
		return false;
	return true;
}

double GRIIODeviceSource::readSampleRate()
{
	char buffer[20];
	bool ok = false;
	double sr;
	if(!sampleRateAvailable())
		return -1;

	iio_device_attr_read(m_iioDev, m_sampleRateAttribute.toStdString().c_str(), buffer, 20);
	QString str(buffer);
	sr = str.toDouble(&ok);
	if(ok) {
		return sr;
	} else {
		return -1;
	}
}

void GRIIODeviceSource::matchChannelToBlockOutputs(GRTopBlock *top)
{
	QMap<GRIIOChannel *, int> map;
	for(GRIIOChannel *ch : qAsConst(m_list)) {
		GRIIOFloatChannelSrc *floatCh = dynamic_cast<GRIIOFloatChannelSrc *>(ch);
		if(floatCh) {
			auto start_sptr = floatCh->getGrStartPoint();

			int idx = getOutputIndex(floatCh->getChannelName());
			int mapIdx = map.value(ch, 0);
			top->connect(src, idx, start_sptr[mapIdx], 0);
			mapIdx++;
			map.insert(ch, mapIdx);
		}

		GRIIOComplexChannelSrc *complexCh = dynamic_cast<GRIIOComplexChannelSrc *>(ch);
		if(complexCh) {
			auto start_sptr = complexCh->getGrStartPoint();
			int idxI = getOutputIndex(complexCh->getChannelNameI());
			int idxQ = getOutputIndex(complexCh->getChannelNameQ());
			int mapIdx = map.value(ch, 0);
			top->connect(src, idxI, start_sptr[mapIdx], 0);
			top->connect(src, idxQ, start_sptr[mapIdx + 1], 0);
			mapIdx += 2;
			map.insert(ch, mapIdx);
		}
	}
}

void GRIIODeviceSource::build_blks(GRTopBlock *top)
{
	qDebug(SCOPY_GR_UTIL) << "Building GRIIODeviceSource";
	if(m_list.count() == 0)
		return;

	computeChannelNames();
	// create block
	src = gr::iio::device_source::make_from(m_ctx, m_deviceName.toStdString(), m_channelNames,
						m_phyDeviceName.toStdString(), gr::iio::iio_param_vec_t(),
						m_buffersize);
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

void GRIIODeviceSource::connect_blk(GRTopBlock *top, GRProxyBlock *) { matchChannelToBlockOutputs(top); }

void GRIIODeviceSource::disconnect_blk(GRTopBlock *top) { top->getGrBlock()->disconnect(src); }

void GRIIODeviceSource::addChannel(GRIIOChannel *ch) { m_list.append(ch); }

void GRIIODeviceSource::removeChannel(GRIIOChannel *ch) { m_list.removeAll(ch); }

unsigned int GRIIODeviceSource::getBuffersize() const { return m_buffersize; }

void GRIIODeviceSource::setBuffersize(unsigned int newBuffersize)
{
	m_buffersize = newBuffersize;
	Q_EMIT requestRebuild();
}

std::vector<std::string> GRIIODeviceSource::channelNames() const { return m_channelNames; }

QString GRIIODeviceSource::deviceName() const { return m_deviceName; }
