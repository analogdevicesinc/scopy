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

#ifndef GRIIODEVICESOURCE_H
#define GRIIODEVICESOURCE_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <iio.h>

#include <gnuradio/iio/device_source.h>

namespace scopy::grutil {

class GRIIODeviceSource;
// typedef GRProxyBlock GRIIOChannel; // TODO: create interface here
class SCOPY_GR_UTIL_EXPORT GRIIOChannel : public GRProxyBlock
{
public:
	GRIIOChannel(QString channelName, GRIIODeviceSource *dev, QObject *parent = nullptr)
		: GRProxyBlock(parent)
		, channelName(channelName)
		, m_dev(dev)
	{}
	GRIIODeviceSource *getDeviceSrc() { return m_dev; }
	QString getChannelName() { return channelName; }
	virtual bool samplerateAttributeAvailable() { return false; }
	virtual double readSampleRate() { return -1; }
	static QString findAttribute(QStringList possibleNames, iio_channel *);

	virtual bool scaleAttributeAvailable() { return false; };
	virtual double readScale() { return 1; };

protected:
	QString channelName;
	GRIIODeviceSource *m_dev;
};

class SCOPY_GR_UTIL_EXPORT GRIIODeviceSource : public GRProxyBlock
{ // is this a proxy block
public:
	GRIIODeviceSource(iio_context *ctx, QString deviceName, QString phyDeviceName, unsigned int buffersize = 0x400,
			  QObject *parent = nullptr);
	void build_blks(GRTopBlock *top) override;
	void destroy_blks(GRTopBlock *top) override;

	void connect_blk(GRTopBlock *top, GRProxyBlock *) override;
	void disconnect_blk(GRTopBlock *top) override;

	void addChannel(GRIIOChannel *);
	void removeChannel(GRIIOChannel *);

	unsigned int getBuffersize() const;
	void setBuffersize(unsigned int newBuffersize);

	std::vector<std::string> channelNames() const;

	QString deviceName() const;
	bool sampleRateAvailable();
	double readSampleRate();

	static QString findAttribute(QStringList possibleNames, iio_device *dev);

	iio_device *iioDev() const;
	iio_context *ctx() const;

protected:
	QList<GRIIOChannel *> m_list;
	std::vector<std::string> m_channelNames;
	QString m_deviceName;
	QString m_phyDeviceName;
	iio_context *m_ctx;
	iio_device *m_iioDev;
	unsigned int m_buffersize;

	gr::iio::device_source::sptr src;

private:
	QString m_sampleRateAttribute;
	void computeChannelNames();
	void addChannelAtIndex(iio_device *dev, QString channelName);
	void matchChannelToBlockOutputs(GRTopBlock *top);
	int getOutputIndex(QString ch);
};
} // namespace scopy::grutil
#endif // GRIIODEVICESOURCE_H
