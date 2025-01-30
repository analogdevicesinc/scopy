/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 */

#ifndef IIOSTREAM_H
#define IIOSTREAM_H

#include "iiocpp/iioresult.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOStream : public QObject
{
	Q_OBJECT
protected:
	IIOStream(QObject *parent = nullptr);
	~IIOStream();

public:
	IIOStream(const IIOStream &) = delete;
	IIOStream &operator=(const IIOStream &) = delete;

	static IIOStream *GetInstance();

	static IIOResult<iio_stream *> create_stream(iio_buffer *buffer, size_t nb_blocks, size_t samples_count);
	static void destroy(iio_stream *stream);
	static IIOResult<const iio_block *> get_next_block(iio_stream *stream);

private:
	static IIOStream *pinstance_;
};
} // namespace scopy

#endif // IIOSTREAM_H
