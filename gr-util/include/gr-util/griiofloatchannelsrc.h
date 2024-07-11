#ifndef GRIIOFLOATCHANNELSRC_H
#define GRIIOFLOATCHANNELSRC_H

#include "griiodevicesource.h"
#include "iioutil/iiounits.h"
#include "scopy-gr-util_export.h"
#include <gnuradio/blocks/stream_to_vector.h>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRIIOFloatChannelSrc : public GRIIOChannel
{
public:
	GRIIOFloatChannelSrc(GRIIODeviceSource *dev, QString channelName, QObject *parent = nullptr);

	void build_blks(GRTopBlock *top) override;
	void destroy_blks(GRTopBlock *top) override;

	virtual bool samplerateAttributeAvailable() override;
	virtual double readSampleRate() override;

	virtual bool scaleAttributeAvailable() override;
	virtual double readScale() override;

	IIOUnit unit();

	const iio_data_format *getFmt() const;
	struct iio_channel *channel() const;
	struct iio_device *dev() const;
	struct iio_context *ctx() const;

	const QString &scaleAttribute() const;

protected:
	gr::basic_block_sptr x2f;
	gr::blocks::stream_to_vector::sptr s2v;

private:
	GRTopBlock *m_top;
	IIOUnit m_unit;
	const iio_data_format *fmt;
	iio_channel *m_iioCh;
	QString m_sampleRateAttribute;
	QString m_scaleAttribute;
};
} // namespace scopy::grutil
#endif // GRIIOFLOATCHANNELSRC_H
