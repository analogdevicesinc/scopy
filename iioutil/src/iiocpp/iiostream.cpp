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

#include "iiocpp/iiostream.h"
#include "iiocpp/iioresult.h"
#include <QApplication>
#include <iio/iio.h>

using namespace scopy;

IIOStream *IIOStream::pinstance_{nullptr};

IIOStream::IIOStream(QObject *parent)
	: QObject(parent)
{
}

IIOStream::~IIOStream() = default;

IIOStream *IIOStream::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOStream(QApplication::instance());
	}
	return pinstance_;
}

IIOResult<iio_stream *> IIOStream::create_stream(iio_buffer *buffer, size_t nb_blocks, size_t samples_count)
{
	iio_stream *stream = iio_buffer_create_stream(buffer, nb_blocks, samples_count);
	int err = iio_err(stream);

	if(err) {
		return IIOResult<iio_stream *>(err);
	}
	return IIOResult<iio_stream *>(stream);
}

void IIOStream::destroy(iio_stream *stream) { iio_stream_destroy(stream); }

IIOResult<const iio_block *> IIOStream::get_next_block(iio_stream *stream)
{
	const iio_block *block = iio_stream_get_next_block(stream);
	int err = iio_err(block);

	if(err) {
		return IIOResult<const iio_block *>(err);
	}
	return IIOResult<const iio_block *>(block);
}
