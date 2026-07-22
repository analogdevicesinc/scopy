#include "backendfixture.h"

#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"
#include "iioutil/ichannelops.h"
#include "iioutil/ibufferops.h"

using namespace scopy;
using namespace scopy::iio;

class TestBufferOps : public QObject
{
	Q_OBJECT
private slots:
	// Mask create/destroy is pure allocation — no device state, safe to run
	// whenever a backend loads (no live context needed).
	void maskLifecycle();
	void maskLifecycle_data() { dctest::addBackendRows(); }

	// Full open -> refill -> pointer-access -> close cycle. Allocates kernel
	// buffers and does DMA, so it is guarded behind DC_TEST_ALLOW_MUTATION.
	void captureCycle();
	void captureCycle_data() { dctest::addBackendRows(); }
};

void TestBufferOps::maskLifecycle()
{
	DC_REQUIRE_BACKEND();
	IBufferOps *bufOps = b->bufferOps();

	ChannelsMaskHandle mask = bufOps->createChannelsMask(8);
	bufOps->destroyChannelsMask(mask);

	// A zero-width mask is a degenerate but legal request; must not crash.
	ChannelsMaskHandle empty = bufOps->createChannelsMask(0);
	bufOps->destroyChannelsMask(empty);
}

void TestBufferOps::captureCycle()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	IBufferOps *bufOps = b->bufferOps();
	DC_REQUIRE_CONTEXT();
	DC_REQUIRE_MUTATION();

	// Find an input device with at least one scan-element channel we can enable
	// and capture from. Skip cleanly if the test URI has no such device.
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	bool captured = false;
	for(unsigned int i = 0; i < devCount && !captured; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);

		ChannelsMaskHandle mask = bufOps->createChannelsMask(chCount);
		unsigned int enabled = 0;
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			if(chOps->isScanElement(ch) && !chOps->isOutput(ch)) {
				chOps->enable(ch, mask);
				++enabled;
			}
		}
		if(enabled == 0) {
			bufOps->destroyChannelsMask(mask);
			continue;
		}

		const size_t samples = 256;
		BufferHandle buf = bufOps->openBuffer(dev, 0, mask, samples, /*cyclic=*/false);
		if(!buf.ptr) {
			bufOps->destroyChannelsMask(mask);
			continue; // device may not support capture (e.g. no DMA) — try next
		}

		Result<void> r = bufOps->refill(buf);
		if(r) {
			// On a successful refill the buffer pointers must delimit a non-empty,
			// forward-ordered block with a positive stride.
			void *start = bufOps->bufferStart(buf);
			void *end = bufOps->bufferEnd(buf);
			QVERIFY(start != nullptr);
			QVERIFY(end != nullptr);
			QVERIFY(static_cast<char *>(end) >= static_cast<char *>(start));
			QVERIFY(bufOps->bufferStep(buf) > 0);
		} else {
			QVERIFY(r.error().code != 0 || !r.error().message.isEmpty());
		}

		bufOps->closeBuffer(buf);
		bufOps->destroyChannelsMask(mask);
		captured = true;
	}

	ctxOps->destroyContext(ctx);
	if(!captured) {
		QSKIP("no capturable input device at test URI");
	}
}

QTEST_MAIN(TestBufferOps)
#include "tst_bufferops.moc"
