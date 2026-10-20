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

#include "component/inputstream.h"

#include <QVector>

namespace libm2k::analog {
class M2kAnalogIn;
}

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

// Buffered analog-in capture over libm2k. Parented to the "analog-in"
// component::Device; reach it with component::streamAt<component::InputStream>().
class M2kInputStream : public component::InputStream
{
	Q_OBJECT
public:
	static constexpr int HW_CHANNELS = 2;

	M2kInputStream(void *resource, libm2k::analog::M2kAnalogIn *ain, scopy::ICmdExecutor *executor,
		       QObject *parent = nullptr);
	~M2kInputStream() override;

	unsigned kernelBuffers() const { return m_kernelBuffers; }
	Result<void> setKernelBuffers(unsigned n);

	QCoro::Task<CommandResponse<void>> openAsync(const component::StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	QCoro::Task<CommandResponse<void>> refillAsync() override;

	const component::StreamFormat &readFormat() const override { return m_format; }

private:
	void *m_resource;
	libm2k::analog::M2kAnalogIn *m_ain;
	scopy::ICmdExecutor *m_executor;
	unsigned m_kernelBuffers = 4;
	QVector<stream::ChannelScale> m_scales;
	component::StreamFormat m_format;
};

} // namespace scopy::adalm2000
