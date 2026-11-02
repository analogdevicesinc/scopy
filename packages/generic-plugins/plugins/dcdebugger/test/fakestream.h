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

#ifndef DCDEBUGGER_TEST_FAKESTREAM_H
#define DCDEBUGGER_TEST_FAKESTREAM_H

#include <QVector>

#include <component/inputstream.h>
#include <component/outputstream.h>
#include <component/scanelement.h>
#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::component;

namespace scopy::dcdebugger::test {
class FakeScanElement : public ScanElement
{
public:
	using ScanElement::ScanElement;
	bool isEnabled() const override { return m_on; }
	QCoro::Task<CommandResponse<void>> enableAsync(bool en) override
	{
		m_on = en;
		co_return CommandResponse<void>(QUuid());
	}
	bool m_on = false;
};

// Interleaved int16 buffer; sample s of element e is e * 1000 + s.
class FakeInputStream : public InputStream
{
public:
	QList<FakeScanElement *> elements() const
	{
		QList<FakeScanElement *> out;
		for(ScanElement *el : findChildren<ScanElement *>()) {
			out.append(static_cast<FakeScanElement *>(el));
		}
		return out;
	}

	explicit FakeInputStream(int elements)
	{
		for(int e = 0; e < elements; ++e) {
			auto *el = new FakeScanElement(this);
			el->setIndex(e);
			el->setId(QStringLiteral("voltage%1").arg(e));
		}
	}
	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override
	{
		if(failOpens > 0) {
			failOpens--;
			co_return CommandResponse<void>(QUuid(), Unexpected<Error>{Error{-EIO, "buffer open failed"}});
		}
		for(auto *el : elements()) {
			el->m_on = cfg.enabledChannels.contains(int(el->index()));
		}
		m_config = cfg;
		m_open = true;
		opens++;
		Q_EMIT openedChanged(true);
		co_return CommandResponse<void>(QUuid());
	}
	QCoro::Task<CommandResponse<void>> closeAsync() override
	{
		m_open = false;
		Q_EMIT openedChanged(false);
		co_return CommandResponse<void>(QUuid());
	}
	QCoro::Task<CommandResponse<void>> refillAsync() override
	{
		QList<FakeScanElement *> on;
		for(auto *el : elements()) {
			if(el->m_on) {
				on.append(el);
			}
		}
		const int n = int(m_config.samplesCount);
		m_buffer.resize(n * on.size());
		m_format = StreamFormat{};
		m_format.data = m_buffer.data();
		m_format.sampleCount = size_t(n);
		for(int c = 0; c < on.size(); ++c) {
			ChannelFormat cf;
			cf.offset = ptrdiff_t(c * sizeof(int16_t));
			cf.stride = ptrdiff_t(on.size() * sizeof(int16_t));
			m_format.channels.append(cf);
			for(int s = 0; s < n; ++s) {
				m_buffer[s * on.size() + c] = int16_t(on[c]->index() * 1000 + s);
			}
		}
		refills++;
		co_return CommandResponse<void>(QUuid());
	}
	const StreamFormat &readFormat() const override { return m_format; }

	int failOpens = 0;
	int opens = 0;
	int refills = 0;

private:
	QVector<int16_t> m_buffer;
	StreamFormat m_format;
};
// Int16 interleaved write buffer; records what each push carried.
class FakeOutputStream : public OutputStream
{
public:
	explicit FakeOutputStream(int elements, bool canCycle = true)
		: m_canCycle(canCycle)
	{
		for(int e = 0; e < elements; ++e) {
			auto *el = new FakeScanElement(this);
			el->setIndex(e);
			el->setId(QStringLiteral("voltage%1").arg(e));
			el->setIsOutput(true);
		}
	}
	QList<FakeScanElement *> elements() const
	{
		QList<FakeScanElement *> out;
		for(ScanElement *el : findChildren<ScanElement *>()) {
			out.append(static_cast<FakeScanElement *>(el));
		}
		return out;
	}
	bool supportsCyclic() const override { return m_canCycle; }
	bool cyclic() const override { return m_cyclic; }
	Result<void> setCyclic(bool c) override
	{
		if(c && !m_canCycle) {
			return Result<void>(Unexpected<Error>{Error{-ENOTSUP, "no"}});
		}
		m_cyclic = c;
		return Result<void>();
	}
	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override
	{
		int on = 0;
		for(auto *el : elements()) {
			el->m_on = cfg.enabledChannels.contains(int(el->index()));
			on += el->m_on ? 1 : 0;
		}
		m_config = cfg;
		m_buffer.fill(0, int(cfg.samplesCount) * on);
		m_format = StreamFormat{};
		m_format.data = m_buffer.data();
		m_format.sampleCount = cfg.samplesCount;
		for(int c = 0; c < on; ++c) {
			ChannelFormat cf;
			cf.offset = ptrdiff_t(c * sizeof(int16_t));
			cf.stride = ptrdiff_t(on * sizeof(int16_t));
			m_format.channels.append(cf);
		}
		m_open = true;
		opens++;
		Q_EMIT openedChanged(true);
		co_return CommandResponse<void>(QUuid());
	}
	QCoro::Task<CommandResponse<void>> closeAsync() override
	{
		m_open = false;
		Q_EMIT openedChanged(false);
		co_return CommandResponse<void>(QUuid());
	}
	StreamFormat &writeFormat() override { return m_format; }
	QCoro::Task<CommandResponse<void>> pushAsync() override
	{
		pushes++;
		lastPush = m_buffer;
		lastPushCyclic = m_cyclic;
		co_return CommandResponse<void>(QUuid());
	}

	int opens = 0;
	int pushes = 0;
	QVector<int16_t> lastPush;
	bool lastPushCyclic = false;

private:
	bool m_canCycle;
	bool m_cyclic = false;
	QVector<int16_t> m_buffer;
	StreamFormat m_format;
};

} // namespace scopy::dcdebugger::test

#endif // DCDEBUGGER_TEST_FAKESTREAM_H
