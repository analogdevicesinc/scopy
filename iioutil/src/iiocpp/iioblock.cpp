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

#include "iiocpp/iioblock.h"
#include <QApplication>

using namespace scopy;

IIOBlock *IIOBlock::pinstance_{nullptr};

IIOBlock::IIOBlock(QObject *parent)
	: QObject(parent)
{
}

IIOBlock::~IIOBlock() = default;

IIOBlock *IIOBlock::GetInstance()
{
	if(!pinstance_) {
		pinstance_ = new IIOBlock(QApplication::instance());
	}
	return pinstance_;
}

IIOResult<iio_block *> IIOBlock::create_block(iio_buffer *buffer, size_t size)
{
	iio_block *block = iio_buffer_create_block(buffer, size);
	int err = iio_err(block);

	if(err) {
		return IIOResult<iio_block *>(err);
	}
	return IIOResult<iio_block *>(block);
}

void IIOBlock::destroy(iio_block *block) { iio_block_destroy(block); }

int IIOBlock::get_dmabuf_fd(const iio_block *block) { return iio_block_get_dmabuf_fd(block); }

int IIOBlock::disable_cpu_access(iio_block *block, bool disable)
{
	return iio_block_disable_cpu_access(block, disable);
}

void *IIOBlock::start(const iio_block *block) { return iio_block_start(block); }

void *IIOBlock::first(const iio_block *block, const iio_channel *chn) { return iio_block_first(block, chn); }

ssize_t IIOBlock::foreach_sample(const iio_block *block, const iio_channels_mask *mask,
				 ssize_t (*callback)(const iio_channel *chn, void *src, size_t bytes, void *d),
				 void *data)
{
	return iio_block_foreach_sample(block, mask, callback, data);
}

int IIOBlock::enqueue(iio_block *block, size_t bytes_used, bool cyclic)
{
	return iio_block_enqueue(block, bytes_used, cyclic);
}

int IIOBlock::dequeue(iio_block *block, bool nonblock) { return iio_block_dequeue(block, nonblock); }

iio_buffer *IIOBlock::get_buffer(const iio_block *block) { return iio_block_get_buffer(block); }
