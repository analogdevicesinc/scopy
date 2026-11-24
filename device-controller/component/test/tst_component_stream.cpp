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

#include "component/backends/iio/iioinputstream.h"
#include "component/backends/iio/iiooutputstream.h"
#include "component/backends/iio/iiochannel.h"
#include "component/stream.h"
#include "component/streamview.h"
#include "component/streamwriter.h"
#include "core/pooledcmdexecutor.h"
#include "fakebufferops.h"

#include <QSignalSpy>
#include <cerrno>
#include <QTest>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

namespace {

scopy::iio::DataFormat s16Format()
{
	scopy::iio::DataFormat f{};
	f.length = 16;
	f.bits = 16;
	f.shift = 0;
	f.is_signed = true;
	f.with_scale = true;
	f.scale = 2.0;
	f.offset = 0.0;
	return f;
}

// Register a fake libiio channel as a stream member, the way the builder does.
IIOChannel *addMember(Stream *stream, dctest::FakeChannel *fc)
{
	auto *chan = new IIOChannel(stream);
	chan->setHandle({fc});
	chan->setId(fc->name);
	chan->setIsOutput(fc->output);
	chan->setStreamIndex(fc->index);
	stream->addChannel(chan);
	return chan;
}

} // namespace

class TstComponentStream : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void openEnablesRefillDecodes();
	void openCopiesConfigBeforeSuspending();
	void membersSortedAndFiltered();
	void disabledMemberLeftOutOfFormat();
	void openRefusesEmptySelection();
	void setEnabledChannelsReopens();
	void setKernelBuffersRefusedWhenOpen();
	void maskFreedOnDelete();
	void outputPushEmits();
	void multiBufferSelector();
	void flattenDoubles();
	void writerRoundTripsThroughView();
	void writerNormalizedScale();
	void cyclicIsAnOutputStreamProperty();
};

void TstComponentStream::openEnablesRefillDecodes()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 4; // two interleaved S16 channels

	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};
	dctest::FakeChannel c1{1, "voltage1", false, true, s16Format()};

	auto *stream = new IIOInputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 2, 0, &exec);
	addMember(stream, &c0);
	addMember(stream, &c1);

	bufOps.chOffset.insert(&c0, 0);
	bufOps.chOffset.insert(&c1, 2);

	QVERIFY(QCoro::waitFor(stream->openAsync({{0, 1}, 3})));
	QVERIFY(stream->isOpen());
	QVERIFY(chOps.isEnabled({&c0}, stream->mask()));
	QVERIFY(chOps.isEnabled({&c1}, stream->mask()));
	QVERIFY(stream->isChannelEnabled(0));
	QVERIFY(stream->isChannelEnabled(1));

	// Fill known bytes: ch0 = 1,2,3 ; ch1 = 10,20,30 (int16 LE, interleaved).
	auto *p = reinterpret_cast<int16_t *>(bufOps.storage.data());
	p[0] = 1;
	p[1] = 10;
	p[2] = 2;
	p[3] = 20;
	p[4] = 3;
	p[5] = 30;

	QSignalSpy ok(stream, &InputStream::refillSucceeded);
	QVERIFY(QCoro::waitFor(stream->refillAsync()));
	QCOMPARE(ok.count(), 1);

	StreamView view(stream->readFormat());
	QCOMPARE(view.channelCount(), 2);
	QCOMPARE(view.sampleCount(), size_t(3));
	// scale = 2.0
	QCOMPARE(view.sampleAsDouble(0, 2), 6.0);
	QCOMPARE(view.sampleAsDouble(1, 0), 20.0);
	const auto all = view.toDoubles();
	QCOMPARE(all[1], (QVector<double>{20.0, 40.0, 60.0}));

	delete stream;
	QCOMPARE(bufOps.masksAlive, 0);
	QVERIFY(bufOps.cancels >= 1);
}

void TstComponentStream::membersSortedAndFiltered()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};
	dctest::FakeChannel c3{3, "voltage3", false, true, s16Format()};

	IIOInputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 4, 0, &exec);
	IIOChannel *m3 = addMember(&stream, &c3);
	IIOChannel *m0 = addMember(&stream, &c0);
	stream.addChannel(m0); // duplicate ignored

	auto *notScan = new IIOChannel(&stream); // streamIndex -1: rejected
	stream.addChannel(notScan);
	stream.addChannel(nullptr);

	QCOMPARE(stream.channels(), (QList<Channel *>{m0, m3}));
	QCOMPARE(stream.channelAt(3), m3);
	QCOMPARE(stream.channelAt(1), nullptr);
}

void TstComponentStream::disabledMemberLeftOutOfFormat()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 4;
	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};
	dctest::FakeChannel c1{1, "voltage1", false, true, s16Format()};

	auto *stream = new IIOInputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 2, 0, &exec);
	addMember(stream, &c0);
	addMember(stream, &c1);
	bufOps.chOffset.insert(&c0, 0);
	bufOps.chOffset.insert(&c1, 2);

	// Enable both, then reopen with only ch1: the mask must drop ch0.
	QVERIFY(QCoro::waitFor(stream->openAsync({{0, 1}, 2})));
	QVERIFY(QCoro::waitFor(stream->closeAsync()));
	QVERIFY(QCoro::waitFor(stream->openAsync({{1}, 2})));
	QVERIFY(!chOps.isEnabled({&c0}, stream->mask()));
	QVERIFY(chOps.isEnabled({&c1}, stream->mask()));
	QVERIFY(!stream->isChannelEnabled(0));

	QVERIFY(QCoro::waitFor(stream->refillAsync()));
	QCOMPARE(stream->readFormat().channels.size(), 1);
	QCOMPARE(stream->readFormat().channels.at(0).streamIndex, 1L);
	QCOMPARE(stream->readFormat().channels.at(0).offset, ptrdiff_t(2));
	delete stream;
}

void TstComponentStream::openRefusesEmptySelection()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 2;
	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};

	IIOInputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 2, 0, &exec);
	addMember(&stream, &c0);

	// Nothing requested, and a non-member index resolves to nothing too.
	for(const QList<int> &req : {QList<int>{}, QList<int>{5}}) {
		const auto resp = QCoro::waitFor(stream.openAsync({req, 4}));
		QVERIFY(!resp);
		QCOMPARE(resp.error().code, -EINVAL);
		QVERIFY(!stream.isOpen());
	}
	QCOMPARE(stream.resolveEnabledChannels({5, 0, 0}), QList<int>({0}));
}

void TstComponentStream::setEnabledChannelsReopens()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 4;
	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};
	dctest::FakeChannel c1{1, "voltage1", false, true, s16Format()};

	IIOInputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 2, 0, &exec);
	addMember(&stream, &c0);
	addMember(&stream, &c1);
	QSignalSpy changed(&stream, &Stream::enabledChannelsChanged);

	// Closed: only recorded.
	QVERIFY(QCoro::waitFor(stream.setEnabledChannelsAsync({1})));
	QVERIFY(!stream.isOpen());
	QCOMPARE(stream.enabledChannels(), QList<int>({1}));
	QCOMPARE(changed.count(), 1);

	QVERIFY(QCoro::waitFor(stream.openAsync({{0}, 8})));
	QCOMPARE(changed.count(), 2);

	// Open: reopened with the new mask and the same samplesCount.
	QVERIFY(QCoro::waitFor(stream.setEnabledChannelsAsync({1, 0})));
	QVERIFY(stream.isOpen());
	QCOMPARE(stream.enabledChannels(), QList<int>({0, 1}));
	QCOMPARE(stream.samplesCount(), size_t(8));
	QVERIFY(chOps.isEnabled({&c0}, stream.mask()));
	QVERIFY(chOps.isEnabled({&c1}, stream.mask()));
	QCOMPARE(changed.count(), 3);

	// Empty: closed and left closed.
	QVERIFY(QCoro::waitFor(stream.setEnabledChannelsAsync({})));
	QVERIFY(!stream.isOpen());
	QVERIFY(stream.enabledChannels().isEmpty());
	QCOMPARE(changed.count(), 4);
}

// The caller's config is not required to outlive the first suspension of openAsync.
void TstComponentStream::openCopiesConfigBeforeSuspending()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 2;
	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};

	auto *stream = new IIOInputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec);
	addMember(stream, &c0);

	StreamConfig cfg{{0}, 3};
	auto task = stream->openAsync(cfg);
	// Suspended on the executor: the caller's object changes (or dies) meanwhile.
	cfg.samplesCount = 999;
	cfg.enabledChannels = {7};
	QVERIFY(QCoro::waitFor(std::move(task)));
	QCOMPARE(stream->samplesCount(), size_t(3));
	QCOMPARE(stream->enabledChannels(), QList<int>({0}));
	delete stream;
}

void TstComponentStream::setKernelBuffersRefusedWhenOpen()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 2;

	dctest::FakeChannel c0{0, "voltage0", false, true, s16Format()};

	IIOInputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec);
	addMember(&stream, &c0);
	QVERIFY(stream.setKernelBuffers(8));
	QVERIFY(QCoro::waitFor(stream.openAsync({{0}, 4})));
	QVERIFY(!stream.setKernelBuffers(2));
}

void TstComponentStream::maskFreedOnDelete()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	{
		IIOInputStream in(&bufOps, &chOps, {}, 2, 0, &exec);
		IIOOutputStream out(&bufOps, &chOps, {}, 2, 0, &exec);
		QCOMPARE(bufOps.masksAlive, 2);
	}
	QCOMPARE(bufOps.masksAlive, 0);
}

void TstComponentStream::outputPushEmits()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 4;

	dctest::FakeChannel c0{0, "voltage0", true, true, s16Format()};
	auto *stream = new IIOOutputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec);
	addMember(stream, &c0);
	bufOps.chOffset.insert(&c0, 0);

	QVERIFY(QCoro::waitFor(stream->openAsync({{0}, 2})));
	QCOMPARE(stream->writeFormat().channels.size(), 1);

	QSignalSpy ok(stream, &OutputStream::pushSucceeded);
	QVERIFY(QCoro::waitFor(stream->pushAsync()));
	QCOMPARE(ok.count(), 1);
	QCOMPARE(bufOps.pushes, 1);

	delete stream;
	QCOMPARE(bufOps.masksAlive, 0);
}

void TstComponentStream::multiBufferSelector()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;

	QObject device;
	// Two input buffers + one output buffer on the same device.
	auto *in0 = new IIOInputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec, &device);
	auto *in1 = new IIOInputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 1, &exec, &device);
	auto *out0 = new IIOOutputStream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec, &device);

	// Enumerate by direction: two inputs, one output.
	QCOMPARE(streamsOf<InputStream>(&device).size(), 2);
	QCOMPARE(streamsOf<OutputStream>(&device).size(), 1);

	// Select by buffer index.
	QCOMPARE(streamAt<InputStream>(&device, 0), static_cast<InputStream *>(in0));
	QCOMPARE(streamAt<InputStream>(&device, 1), static_cast<InputStream *>(in1));
	QVERIFY(streamAt<InputStream>(&device, 2) == nullptr);
	QCOMPARE(streamAt<OutputStream>(&device, 0), static_cast<OutputStream *>(out0));

	QCOMPARE(in0->bufferIndex(), 0u);
	QCOMPARE(in1->bufferIndex(), 1u);
}

void TstComponentStream::flattenDoubles()
{
	// Two interleaved S16 channels, 3 samples, scale = 2.0.
	// ch0 raw = 1,2,3 -> 2,4,6 ; ch1 raw = 10,20,30 -> 20,40,60.
	int16_t storage[] = {1, 10, 2, 20, 3, 30};

	auto s16 = [](ptrdiff_t offset) {
		ChannelFormat cf;
		cf.offset = offset;
		cf.stride = 4; // 2 channels * 2 bytes
		cf.type = DataType::S16;
		cf.validBits = 16;
		cf.shift = 0;
		cf.scale = 2.0;
		cf.offsetPhys = 0.0;
		return cf;
	};

	StreamFormat format;
	format.data = storage;
	format.sampleCount = 3;
	format.layout = SamplesLayout::Interleaved;
	format.channels = {s16(0), s16(2)};

	StreamView view(format);

	// Sanity against per-channel decode.
	QCOMPARE(view.toDoubles(), (QVector<QVector<double>>{{2.0, 4.0, 6.0}, {20.0, 40.0, 60.0}}));

	// Sample-major: [c0s0, c1s0, c0s1, c1s1, c0s2, c1s2].
	QCOMPARE(view.toInterleavedDoubles(), (QVector<double>{2.0, 20.0, 4.0, 40.0, 6.0, 60.0}));

	// Channel-major: [c0s0, c0s1, c0s2, c1s0, c1s1, c1s2].
	QCOMPARE(view.toConcatenatedDoubles(), (QVector<double>{2.0, 4.0, 6.0, 20.0, 40.0, 60.0}));
}

void TstComponentStream::writerRoundTripsThroughView()
{
	// Two interleaved channels: 12-bit signed shifted by 4 in 16-bit, and 8-bit unsigned.
	QByteArray storage(4 * 3, 0);
	StreamFormat f;
	f.data = storage.data();
	f.sampleCount = 4;
	ChannelFormat a;
	a.offset = 0;
	a.stride = 3;
	a.type = DataType::S16;
	a.validBits = 12;
	a.shift = 4;
	ChannelFormat b;
	b.offset = 2;
	b.stride = 3;
	b.type = DataType::U8;
	b.validBits = 8;
	f.channels = {a, b};

	StreamWriter w(f);
	const int64_t as[] = {-2048, -1, 0, 2047};
	const int64_t bs[] = {0, 1, 128, 255};
	for(size_t s = 0; s < 4; ++s) {
		w.setRaw(0, s, as[s]);
		w.setRaw(1, s, bs[s]);
	}
	StreamView v(f);
	for(size_t s = 0; s < 4; ++s) {
		QCOMPARE(v.rawSample(0, s), as[s]);
		QCOMPARE(v.rawSample(1, s), bs[s]);
	}
	// The low (shifted-out) bits stay clear.
	QCOMPARE(uint8_t(storage[0]) & 0x0f, 0);
}

void TstComponentStream::writerNormalizedScale()
{
	ChannelFormat s16;
	s16.type = DataType::S16;
	s16.validBits = 16;
	QCOMPARE(StreamWriter::normalizedToRaw(s16, 1.0), int64_t(32767));
	QCOMPARE(StreamWriter::normalizedToRaw(s16, -1.0), int64_t(-32767));
	QCOMPARE(StreamWriter::normalizedToRaw(s16, 0.0), int64_t(0));
	QCOMPARE(StreamWriter::normalizedToRaw(s16, 5.0), int64_t(32767)); // clamped
	ChannelFormat u8;
	u8.type = DataType::U8;
	u8.validBits = 8;
	QCOMPARE(StreamWriter::normalizedToRaw(u8, -1.0), int64_t(0));
	QCOMPARE(StreamWriter::normalizedToRaw(u8, 1.0), int64_t(255));
}

void TstComponentStream::cyclicIsAnOutputStreamProperty()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	IIOOutputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec);
	OutputStream *out = &stream;
	QVERIFY(out->supportsCyclic());
	QVERIFY(!out->cyclic());
	QVERIFY(out->setCyclic(true));
	QVERIFY(out->property("cyclic").toBool());
	QVERIFY(QCoro::waitFor(out->openAsync({{}, 4})));
	QVERIFY(!out->setCyclic(false)); // refused while open
	QVERIFY(out->cyclic());
}

QTEST_MAIN(TstComponentStream)
#include "tst_component_stream.moc"
