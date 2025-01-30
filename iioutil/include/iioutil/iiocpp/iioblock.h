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

#ifndef IIOBLOCK_H
#define IIOBLOCK_H

#include "iiocpp/iioresult.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIOBlock : public QObject
{
	Q_OBJECT
protected:
	IIOBlock(QObject *parent = nullptr);
	~IIOBlock();

public:
	IIOBlock(const IIOBlock &) = delete;
	IIOBlock &operator=(const IIOBlock &) = delete;

	static IIOBlock *GetInstance();

	static IIOResult<iio_block *> create_block(iio_buffer *buffer, size_t size);
	static void destroy(iio_block *block);
	static int get_dmabuf_fd(const iio_block *block);
	static int disable_cpu_access(iio_block *block, bool disable);
	static void *start(const iio_block *block);
	static void *first(const iio_block *block, const iio_channel *chn);
	static ssize_t foreach_sample(const iio_block *block, const iio_channels_mask *mask,
				      ssize_t (*callback)(const iio_channel *chn, void *src, size_t bytes, void *d),
				      void *data);
	static int enqueue(iio_block *block, size_t bytes_used, bool cyclic);
	static int dequeue(iio_block *block, bool nonblock);
	static iio_buffer *get_buffer(const iio_block *block);

private:
	static IIOBlock *pinstance_;
};
} // namespace scopy

#endif // IIOBLOCK_H
