#ifndef GRIIOFLOATCHANNELSRC_H
#define GRIIOFLOATCHANNELSRC_H

#include "griiodevicesource.h"
#include "scopy-gr-util_export.h"

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRIIOFloatChannelSrc : public GRIIOChannel
{
public:
	GRIIOFloatChannelSrc(GRIIODeviceSource *dev, QString channelName, QObject *parent = nullptr);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

	virtual bool samplerateAttributeAvailable() override;
	virtual double readSampleRate() override;

	virtual bool scaleAttributeAvailable() override;
	virtual double readScale() override;

	const iio_data_format *getFmt() const;
	struct iio_channel *channel() const;
	struct iio_device *dev() const;
	struct iio_context *ctx() const;

	const QString &scaleAttribute() const;

protected:
	gr::basic_block_sptr x2f;

private:
	const iio_data_format *fmt;
	iio_channel *m_iioCh;
	QString m_sampleRateAttribute;
	QString m_scaleAttribute;
};
} // namespace scopy::grutil
#endif // GRIIOFLOATCHANNELSRC_H
