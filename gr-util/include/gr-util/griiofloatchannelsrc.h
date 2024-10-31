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
