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
 */

#pragma once

#include "m2kstreamformat.h"

#include "component/outputstream.h"

#include <QVector>

#include <cstdint>

namespace libm2k::analog {
class M2kAnalogOut;
}

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

// Buffered DAC playback over libm2k. Parented to the "analog-out"
// component::Device; reach it with component::streamAt<component::OutputStream>().
//
// The buffer is owned here and exposed through writeFormat(): fill it between
// openAsync() and pushAsync(), using stream::voltsToRawCode() to encode.
class M2kOutputStream : public component::OutputStream
{
	Q_OBJECT
public:
	static constexpr int HW_CHANNELS = 2;

	M2kOutputStream(void *resource, libm2k::analog::M2kAnalogOut *aout, scopy::ICmdExecutor *executor,
			QObject *parent = nullptr);
	~M2kOutputStream() override;

	bool cyclic() const { return m_cyclic; }
	Result<void> setCyclic(bool enable);

	unsigned kernelBuffers() const { return m_kernelBuffers; }
	Result<void> setKernelBuffers(unsigned n);

	QCoro::Task<CommandResponse<void>> openAsync(const component::StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	QCoro::Task<CommandResponse<void>> pushAsync() override;

	component::StreamFormat &writeFormat() override { return m_format; }

	// Per-channel raw -> volts maps, in the order the channels were requested.
	const QVector<stream::ChannelScale> &scales() const { return m_scales; }

private:
	void *m_resource;
	libm2k::analog::M2kAnalogOut *m_aout;
	scopy::ICmdExecutor *m_executor;
	bool m_cyclic = true;
	unsigned m_kernelBuffers = 4;
	QVector<int16_t> m_buffer;
	QVector<stream::ChannelScale> m_scales;
	component::StreamFormat m_format;
};

} // namespace scopy::adalm2000
