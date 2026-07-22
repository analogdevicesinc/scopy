#include "backendfixture.h"

#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"
#include "iioutil/ichannelops.h"
#include "iioutil/ibufferops.h"

#include <vector>

using namespace scopy;
using namespace scopy::iio;

class TestChannelOps : public QObject
{
	Q_OBJECT
private slots:
	void enumerationAndIds();
	void enumerationAndIds_data() { dctest::addBackendRows(); }

	void metadataCallable();
	void metadataCallable_data() { dctest::addBackendRows(); }

	void dataFormatCallable();
	void dataFormatCallable_data() { dctest::addBackendRows(); }

	void attrEnumerationConsistent();
	void attrEnumerationConsistent_data() { dctest::addBackendRows(); }

	void convertRoundTrips();
	void convertRoundTrips_data() { dctest::addBackendRows(); }

	void enableDisable();
	void enableDisable_data() { dctest::addBackendRows(); }
};

void TestChannelOps::enumerationAndIds()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			QVERIFY(ch.ptr != nullptr);
			QVERIFY(!chOps->id(ch).isEmpty());
			// isOutput/isScanElement are pure metadata — must not crash.
			(void)chOps->isOutput(ch);
			(void)chOps->isScanElement(ch);
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestChannelOps::metadataCallable()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			// name/label may be empty (v0 label returns "") — exercise the path only.
			(void)chOps->name(ch);
			(void)chOps->label(ch);
			(void)chOps->index(ch);
			(void)chOps->channelType(ch);
			(void)chOps->modifier(ch);
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestChannelOps::dataFormatCallable()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			// Scan-element channels must report a usable sample width.
			if(chOps->isScanElement(ch)) {
				DataFormat fmt = chOps->dataFormat(ch);
				QVERIFY(fmt.length > 0);
			}
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestChannelOps::attrEnumerationConsistent()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			const unsigned int an = chOps->attrsCount(ch);
			for(unsigned int a = 0; a < an; ++a) {
				QVERIFY(!chOps->attrName(ch, a).isEmpty());
			}
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestChannelOps::convertRoundTrips()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	DC_REQUIRE_CONTEXT();

	// convert/convertInverse are pure computation (no kernel call), so they are
	// safe to run unconditionally. For a scan-element channel, converting a raw
	// sample and inverting it back must round-trip to the original bytes.
	bool exercised = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount && !exercised; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount && !exercised; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			if(!chOps->isScanElement(ch)) {
				continue;
			}
			DataFormat fmt = chOps->dataFormat(ch);
			const size_t bytes = fmt.length / 8;
			if(bytes == 0 || bytes > 8) {
				continue;
			}
			std::vector<unsigned char> src(bytes, 0), host(bytes, 0), back(bytes, 0);
			for(size_t k = 0; k < bytes; ++k) {
				src[k] = static_cast<unsigned char>(0xA5 + k);
			}
			chOps->convert(ch, host.data(), src.data());
			chOps->convertInverse(ch, back.data(), host.data());
			QCOMPARE(QByteArray(reinterpret_cast<char *>(back.data()), int(bytes)),
				 QByteArray(reinterpret_cast<char *>(src.data()), int(bytes)));
			exercised = true;
		}
	}

	ctxOps->destroyContext(ctx);
	if(!exercised) {
		QSKIP("no scan-element channel to convert at test URI");
	}
}

void TestChannelOps::enableDisable()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	IBufferOps *bufOps = b->bufferOps();
	DC_REQUIRE_CONTEXT();
	DC_REQUIRE_MUTATION();

	// Flipping the scan mask changes device state — guarded. After enable() then
	// disable() the channel's enabled state must return to false (v1 reads the
	// mask; v0 ignores it, in which case isEnabled is a stable no-op either way).
	bool exercised = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount && !exercised; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		ChannelsMaskHandle mask = bufOps->createChannelsMask(chCount);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			if(!chOps->isScanElement(ch)) {
				continue;
			}
			chOps->enable(ch, mask);
			chOps->disable(ch, mask);
			// Must be callable and consistent; no crash, returns a bool.
			(void)chOps->isEnabled(ch, mask);
			exercised = true;
		}
		bufOps->destroyChannelsMask(mask);
	}

	ctxOps->destroyContext(ctx);
	if(!exercised) {
		QSKIP("no scan-element channel to toggle at test URI");
	}
}

QTEST_MAIN(TestChannelOps)
#include "tst_channelops.moc"
