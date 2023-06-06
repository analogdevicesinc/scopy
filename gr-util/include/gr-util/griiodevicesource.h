#ifndef GRIIODEVICESOURCE_H
#define GRIIODEVICESOURCE_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"
#include <gnuradio/iio/device_source.h>
#include <iio.h>

namespace scopy::grutil {

class GRIIODeviceSource;
//typedef GRProxyBlock GRIIOChannel; // TODO: create interface here
class SCOPY_GR_UTIL_EXPORT GRIIOChannel : public GRProxyBlock {
public:
	GRIIOChannel(QString channelName, GRIIODeviceSource *dev, QObject *parent = nullptr) : GRProxyBlock(parent), channelName(channelName), dev(dev) { }
	GRIIODeviceSource* getDeviceSrc() { return dev; }
	QString getChannelName() { return channelName;}

protected:

	QString channelName;
	GRIIODeviceSource* dev;

};

class SCOPY_GR_UTIL_EXPORT GRIIODeviceSource : public GRProxyBlock { // is this a proxy block
public:
	GRIIODeviceSource(iio_context *ctx, QString deviceName, QString phyDeviceName, unsigned int buffersize = 0x400, QObject *parent = nullptr);
	void build_blks(GRTopBlock *top) override;
	void destroy_blks(GRTopBlock *top) override;

	void connect_blk(GRTopBlock *top, GRProxyBlock*) override;
	void disconnect_blk(GRTopBlock *top) override;

	void addChannel(GRIIOChannel*);
	void removeChannel(GRIIOChannel*);

	unsigned int getBuffersize() const;
	void setBuffersize(unsigned int newBuffersize);

	std::vector<std::string> channelNames() const;

	QString deviceName() const;

protected:
	QList<GRIIOChannel*> m_list;
	std::vector<std::string> m_channelNames;
	QString m_deviceName;
	QString m_phyDeviceName;
	iio_context *m_ctx;
	unsigned int m_buffersize;

	gr::iio::device_source::sptr src;

private:
	void computeChannelNames();
	void addChannelAtIndex(iio_device *dev, QString channelName);
	void matchChannelToBlockOutputs(GRTopBlock* top);
	int getOutputIndex(QString ch);
};
}
#endif // GRIIODEVICESOURCE_H
