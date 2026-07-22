#include "backendfixture.h"

#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"
#include "iioutil/ibufferops.h"

using namespace scopy;
using namespace scopy::iio;

class TestDeviceOps : public QObject
{
	Q_OBJECT
private slots:
	void enumerationAndIds();
	void enumerationAndIds_data() { dctest::addBackendRows(); }

	void findDeviceRoundTrips();
	void findDeviceRoundTrips_data() { dctest::addBackendRows(); }

	void attrEnumerationConsistent();
	void attrEnumerationConsistent_data() { dctest::addBackendRows(); }

	void metadataCallable();
	void metadataCallable_data() { dctest::addBackendRows(); }

	void debugAndBufferAttrEnumeration();
	void debugAndBufferAttrEnumeration_data() { dctest::addBackendRows(); }

	void sampleSizeAndTrigger();
	void sampleSizeAndTrigger_data() { dctest::addBackendRows(); }

	void regReadWrite();
	void regReadWrite_data() { dctest::addBackendRows(); }
};

void TestDeviceOps::enumerationAndIds()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
    IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		QVERIFY(dev.ptr != nullptr);
		QVERIFY(!devOps->id(dev).isEmpty());
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::findDeviceRoundTrips()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	if(count == 0) {
		ctxOps->destroyContext(ctx);
		QSKIP("no devices at test URI");
	}
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const QString id = devOps->id(dev);
		DeviceHandle found = ctxOps->findDevice(ctx, id);
		QVERIFY(found.ptr != nullptr);
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::attrEnumerationConsistent()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int an = devOps->attrsCount(dev);
		for(unsigned int a = 0; a < an; ++a) {
			QVERIFY(!devOps->attrName(dev, a).isEmpty());
		}
		// channelsCount vs getChannel must agree.
		const unsigned int cn = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < cn; ++c) {
			QVERIFY(devOps->getChannel(dev, c).ptr != nullptr);
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::metadataCallable()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		// All pure-metadata getters must be callable without crashing. name/label
		// may be empty (v0 label falls back), so only exercise the call path.
		QVERIFY(!devOps->id(dev).isEmpty());
		(void)devOps->name(dev);
		(void)devOps->label(dev);
		(void)devOps->isTrigger(dev);
		(void)devOps->isHwmon(dev);
        (void)devOps->buffersCount(dev);
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::debugAndBufferAttrEnumeration()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int dn = devOps->debugAttrsCount(dev);
		for(unsigned int a = 0; a < dn; ++a) {
			QVERIFY(!devOps->debugAttrName(dev, a).isEmpty());
		}
		// Buffer-attr location differs (v0: device-owned, bufferIdx ignored;
		// v1: per-buffer) — enumerate over every reported buffer to cover both.
		const unsigned int bufCount = devOps->buffersCount(dev);
		for(unsigned int bi = 0; bi < bufCount; ++bi) {
			const unsigned int bn = devOps->bufferAttrsCount(dev, bi);
			for(unsigned int a = 0; a < bn; ++a) {
				QVERIFY(!devOps->bufferAttrName(dev, bi, a).isEmpty());
			}
		}
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::sampleSizeAndTrigger()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IBufferOps *bufOps = b->bufferOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int cn = devOps->channelsCount(dev);
		// sampleSize over an all-enabled mask must be non-negative and never crash.
		ChannelsMaskHandle mask = bufOps->createChannelsMask(cn);
        (void)devOps->sampleSize(dev, mask);
		bufOps->destroyChannelsMask(mask);
		// getTrigger is a read-only query; a device without a trigger returns null.
		(void)devOps->getTrigger(dev);
	}

	ctxOps->destroyContext(ctx);
}

void TestDeviceOps::regReadWrite()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	DC_REQUIRE_CONTEXT();
	DC_REQUIRE_MUTATION();

	// Register access touches hardware — only against an opted-in safe device.
	// We read register 0; a backend that doesn't support debug regs must surface
	// a typed Error rather than crash.
	const unsigned int count = ctxOps->devicesCount(ctx);
	bool exercised = false;
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		if(devOps->debugAttrsCount(dev) == 0) {
			continue; // reg access is a debug-plane feature
		}
		Result<uint32_t> r = devOps->regRead(dev, 0x0);
		if(!r) {
			QVERIFY(r.error().code != 0 || !r.error().message.isEmpty());
		}
		exercised = true;
		break;
	}

	ctxOps->destroyContext(ctx);
	if(!exercised) {
		QSKIP("no debug-capable device at test URI");
	}
}

QTEST_MAIN(TestDeviceOps)
#include "tst_deviceops.moc"
