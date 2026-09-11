/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#pragma once

#include "core/resultcommand.h"
#include "iioutil/ibufferops.h"

#include <cerrno>
#include "iioutil/controller_iioutils_export.h"

namespace scopy::iio {

class CONTROLLER_IIOUTILS_EXPORT BufferOpenCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	BufferOpenCommand(IBufferOps *ops, DeviceHandle dev, unsigned int idx, ChannelsMaskHandle mask,
			  size_t samplesCount, bool cyclic, unsigned int kernelBuffers = 4, QObject *parent = nullptr)
		: ResultCommand(dev.ptr, parent)
		, m_ops(ops)
		, m_devHandle(dev)
		, m_idx(idx)
		, m_mask(mask)
		, m_samples(samplesCount)
		, m_cyclic(cyclic)
		, m_kernelBuffers(kernelBuffers)
	{}

	BufferHandle openedHandle() const { return m_openedHandle; }

protected:
	void run() override
	{
		m_openedHandle = m_ops->openBuffer(m_devHandle, m_idx, m_mask, m_samples, m_cyclic, m_kernelBuffers);
		setResult(m_openedHandle.ptr
				  ? Result<void>()
				  : Result<void>(Unexpected{Error{-EIO, QStringLiteral("buffer open failed")}}));
	}

private:
	IBufferOps *m_ops;
	DeviceHandle m_devHandle;
	unsigned int m_idx;
	ChannelsMaskHandle m_mask;
	size_t m_samples;
	bool m_cyclic;
	unsigned int m_kernelBuffers;
	BufferHandle m_openedHandle;
};

} // namespace scopy::iio
