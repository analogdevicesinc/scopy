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

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/inputstream.h>
#include <component/outputstream.h>
#include <component/channel.h>
#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::component;

namespace scopy::dcdebugger::test {
// Member channels "voltage<e>" with streamIndex e, owned by the stream.
inline void addFakeChannels(Stream *stream, int count, bool output)
{
	for(int e = 0; e < count; ++e) {
		auto *ch = new Channel(stream);
		ch->setStreamIndex(e);
		ch->setId(QStringLiteral("voltage%1").arg(e));
		ch->setIsOutput(output);
		stream->addChannel(ch);
	}
}

// Interleaved int16 buffer; sample s of channel e is e * 1000 + s.
class FakeInputStream : public InputStream
{
public:
	explicit FakeInputStream(int elements) { addFakeChannels(this, elements, false); }
	QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) override
	{
		if(failOpens > 0) {
			failOpens--;
			co_return CommandResponse<void>(QUuid(), Unexpected<Error>{Error{-EIO, "buffer open failed"}});
		}
		commitConfig(StreamConfig{resolveEnabledChannels(cfg.enabledChannels), cfg.samplesCount});
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
		const QList<int> on = enabledChannels();
		const int n = int(m_config.samplesCount);
		m_buffer.resize(n * on.size());
		m_format = StreamFormat{};
		m_format.data = m_buffer.data();
		m_format.sampleCount = size_t(n);
		for(int c = 0; c < on.size(); ++c) {
			ChannelFormat cf;
			cf.streamIndex = on[c];
			cf.offset = ptrdiff_t(c * sizeof(int16_t));
			cf.stride = ptrdiff_t(on.size() * sizeof(int16_t));
			m_format.channels.append(cf);
			for(int s = 0; s < n; ++s) {
				m_buffer[s * on.size() + c] = int16_t(on[c] * 1000 + s);
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
// Returns `value` on every read (as a real reader, completes after a suspension-free co_return).
class FakeAttributeReader : public AttributeReader
{
public:
	QCoro::Task<CommandResponse<QByteArray>> readAsync(size_t bytes = 4096) override
	{
		Q_UNUSED(bytes);
		reads++;
		if(fail) {
			co_return CommandResponse<QByteArray>(QUuid(), Unexpected<Error>{Error{-EIO, "read failed"}});
		}
		Result<QByteArray> r(value.toUtf8());
		Q_EMIT readSucceeded(r);
		co_return CommandResponse<QByteArray>(QUuid(), r);
	}
	QString value = "0";
	bool fail = false;
	int reads = 0;
};

inline Attribute *makeAttribute(QObject *parent, const QString &name, const QString &value,
				FakeAttributeReader **reader = nullptr)
{
	auto *a = new Attribute(parent);
	a->setName(name);
	auto *r = new FakeAttributeReader();
	r->value = value;
	a->addReadCapability(r);
	if(reader) {
		*reader = r;
	}
	return a;
}

// Int16 interleaved write buffer; records what each push carried.
class FakeOutputStream : public OutputStream
{
public:
	explicit FakeOutputStream(int elements, bool canCycle = true)
		: m_canCycle(canCycle)
	{
		addFakeChannels(this, elements, true);
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
		commitConfig(StreamConfig{resolveEnabledChannels(cfg.enabledChannels), cfg.samplesCount});
		const QList<int> enabled = enabledChannels();
		const int on = int(enabled.size());
		m_buffer.fill(0, int(cfg.samplesCount) * on);
		m_format = StreamFormat{};
		m_format.data = m_buffer.data();
		m_format.sampleCount = cfg.samplesCount;
		for(int c = 0; c < on; ++c) {
			ChannelFormat cf;
			cf.streamIndex = enabled[c];
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
