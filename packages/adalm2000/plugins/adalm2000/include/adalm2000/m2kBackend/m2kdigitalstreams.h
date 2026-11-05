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
#include "component/outputstream.h"

#include <QVector>

#include <cstdint>

namespace libm2k::digital {
class M2kDigital;
}

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

// Buffered logic capture. Parented to the "digital" component::Device.
//
// The hardware always samples all 16 pins into one word, and there is no
// per-channel input enable in libm2k, so StreamConfig::enabledChannels selects
// which pins the format exposes rather than changing what is captured.
class M2kDigitalInputStream : public component::InputStream
{
	Q_OBJECT
public:
	M2kDigitalInputStream(void *resource, libm2k::digital::M2kDigital *digital, scopy::ICmdExecutor *executor,
			      QObject *parent = nullptr);
	~M2kDigitalInputStream() override;

	unsigned kernelBuffers() const { return m_kernelBuffers; }
	Result<void> setKernelBuffers(unsigned n);

	QCoro::Task<CommandResponse<void>> openAsync(const component::StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	QCoro::Task<CommandResponse<void>> refillAsync() override;

	const component::StreamFormat &readFormat() const override { return m_format; }

private:
	void *m_resource;
	libm2k::digital::M2kDigital *m_digital;
	scopy::ICmdExecutor *m_executor;
	unsigned m_kernelBuffers = 4;
	component::StreamFormat m_format;
};

// Buffered pattern playback. Parented to the "digital" component::Device.
//
// CAUTION: openAsync() drives the requested pins to OUTPUT. The Digital I/O tool
// tracks pin direction in its own dio::PinState shadow, which this bypasses, so
// that tool's displayed direction goes stale until it re-reads. Same class of
// problem as the D21 shadow-reader note; arbitration is a controller's job, not
// a stream's.
class M2kDigitalOutputStream : public component::OutputStream
{
	Q_OBJECT
public:
	M2kDigitalOutputStream(void *resource, libm2k::digital::M2kDigital *digital, scopy::ICmdExecutor *executor,
			       QObject *parent = nullptr);
	~M2kDigitalOutputStream() override;

	// libm2k's digital setCyclic() is device-global, not per channel.
	bool cyclic() const { return m_cyclic; }
	Result<void> setCyclic(bool enable);

	QCoro::Task<CommandResponse<void>> openAsync(const component::StreamConfig &cfg) override;
	QCoro::Task<CommandResponse<void>> closeAsync() override;
	QCoro::Task<CommandResponse<void>> pushAsync() override;

	component::StreamFormat &writeFormat() override { return m_format; }

private:
	void *m_resource;
	libm2k::digital::M2kDigital *m_digital;
	scopy::ICmdExecutor *m_executor;
	bool m_cyclic = true;
	QVector<uint16_t> m_buffer;
	component::StreamFormat m_format;
};

} // namespace scopy::adalm2000
