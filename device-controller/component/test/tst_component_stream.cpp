#include "component/backends/iio/iioinputstream.h"
#include "component/backends/iio/iiooutputstream.h"
#include "component/backends/iio/iioscanelement.h"
#include "component/stream.h"
#include "component/streamview.h"
#include "core/pooledcmdexecutor.h"
#include "fakebufferops.h"

#include <QSignalSpy>
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

} // namespace

class TstComponentStream : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void openEnablesRefillDecodes();
	void setKernelBuffersRefusedWhenOpen();
	void maskFreedOnDelete();
	void outputPushEmits();
	void multiBufferSelector();
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
	auto *e0 = new IIOScanElement(&chOps, {&c0}, stream->mask(), &exec, stream);
	e0->setIndex(0);
	auto *e1 = new IIOScanElement(&chOps, {&c1}, stream->mask(), &exec, stream);
	e1->setIndex(1);

	bufOps.chOffset.insert(&c0, 0);
	bufOps.chOffset.insert(&c1, 2);

    QVERIFY(QCoro::waitFor(stream->openAsync({{0, 1}, 3})));
	QVERIFY(stream->isOpen());
	QVERIFY(e0->isEnabled());
	QVERIFY(e1->isEnabled());

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

void TstComponentStream::setKernelBuffersRefusedWhenOpen()
{
	PooledCmdExecutor exec(1);
	dctest::FakeChannelOps chOps;
	dctest::FakeBufferOps bufOps;
	bufOps.step = 2;

	IIOInputStream stream(&bufOps, &chOps, {reinterpret_cast<void *>(0x1)}, 1, 0, &exec);
	QVERIFY(stream.setKernelBuffers(8));
    QVERIFY(QCoro::waitFor(stream.openAsync({{}, 4})));
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
	auto *e0 = new IIOScanElement(&chOps, {&c0}, stream->mask(), &exec, stream);
	e0->setIndex(0);
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

QTEST_MAIN(TstComponentStream)
#include "tst_component_stream.moc"
