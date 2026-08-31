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

#include "component/outputstream.h"
#include "iioutil/handles.h"

namespace scopy {
class ICmdExecutor;
namespace iio {
class IBufferOps;
class IChannelOps;
class BufferPushCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOOutputStream : public OutputStream
{
	Q_OBJECT
	Q_PROPERTY(unsigned kernelBuffers READ kernelBuffers CONSTANT)
	Q_PROPERTY(bool cyclic READ cyclic CONSTANT)
public:
	IIOOutputStream(scopy::iio::IBufferOps *bufOps, scopy::iio::IChannelOps *chOps, scopy::iio::DeviceHandle dev,
			int nbChannels, unsigned bufferIndex, scopy::ICmdExecutor *executor, QObject *parent = nullptr);
	~IIOOutputStream() override;

	scopy::iio::ChannelsMaskHandle mask() const { return m_mask; }
	unsigned kernelBuffers() const { return m_kernelBuffers; }
	bool cyclic() const { return m_cyclic; }
	Result<void> setKernelBuffers(unsigned n);
	Result<void> setCyclic(bool cyclic);

	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	StreamFormat &writeFormat() override { return m_format; }
	QCoro::Task<CommandResponse<void>> pushAsync() override;

private:
	QCoro::Task<void> applyEnabledChannels(const QList<int> &channels);
	void buildStreamFormat();

	scopy::iio::IBufferOps *m_bufOps;
	scopy::iio::IChannelOps *m_chOps;
	scopy::iio::DeviceHandle m_dev;
	scopy::iio::ChannelsMaskHandle m_mask;
	scopy::iio::BufferHandle m_buffer;
	scopy::ICmdExecutor *m_executor;
	unsigned m_kernelBuffers = 4;
	bool m_cyclic = false;
	StreamFormat m_format;
};

} // namespace scopy::component::iio
