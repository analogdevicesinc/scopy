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

#ifndef GRIIOCOMPLEXCHANNELSRC_H
#define GRIIOCOMPLEXCHANNELSRC_H

#include "griiodevicesource.h"
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/blocks/stream_to_vector.h>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT GRIIOComplexChannelSrc : public GRIIOChannel
{
public:
	GRIIOComplexChannelSrc(QString channelName, GRIIODeviceSource *dev, QString channelNameI, QString channelNameQ,
			       QObject *parent = nullptr);

	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

	const QString &getChannelNameI() const;
	const QString &getChannelNameQ() const;

	const iio_data_format *getFmt() const;

protected:
	QString channelNameI;
	QString channelNameQ;

	const iio_data_format *fmt;
	gr::blocks::short_to_float::sptr s2f[2];
	gr::blocks::float_to_complex::sptr f2c;
	gr::blocks::stream_to_vector::sptr s2v;
};
} // namespace scopy::grutil
#endif // GRIIOCOMPLEXCHANNELSRC_H
