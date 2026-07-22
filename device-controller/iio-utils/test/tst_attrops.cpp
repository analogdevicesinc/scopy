#include "backendfixture.h"

#include "iioutil/icontextops.h"
#include "iioutil/ideviceops.h"
#include "iioutil/ichannelops.h"
#include "iioutil/iattrops.h"

using namespace scopy;
using namespace scopy::iio;

class TestAttrOps : public QObject
{
	Q_OBJECT
private slots:
	void deviceAttrRead();
	void deviceAttrRead_data() { dctest::addBackendRows(); }

	void channelAttrRead();
	void channelAttrRead_data() { dctest::addBackendRows(); }

	void rangeAndAvailableParse();
	void rangeAndAvailableParse_data() { dctest::addBackendRows(); }

	void attrWrite();
	void attrWrite_data() { dctest::addBackendRows(); }
};

void TestAttrOps::deviceAttrRead()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IAttrOps *attrOps = b->attrOps();
	DC_REQUIRE_CONTEXT();

	bool readAny = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount && !readAny; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int an = devOps->attrsCount(dev);
		if(an == 0) {
			continue;
		}
		const QString aname = devOps->attrName(dev, 0);
		AttrHandle h = attrOps->deviceAttr(dev, aname);
		QVERIFY(h.ptr != nullptr);
		Result<QByteArray> r = attrOps->read(h);
		if(!r) {
			// A failure must be a typed Error (errno and/or message), never a crash.
			QVERIFY(r.error().code != 0 || !r.error().message.isEmpty());
		}
		attrOps->releaseAttr(h);
		readAny = true;
	}

	ctxOps->destroyContext(ctx);
	if(!readAny) {
		QSKIP("no device attributes at test URI");
	}
}

void TestAttrOps::channelAttrRead()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	IAttrOps *attrOps = b->attrOps();
	DC_REQUIRE_CONTEXT();

	bool readAny = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount && !readAny; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount && !readAny; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			const unsigned int an = chOps->attrsCount(ch);
			if(an == 0) {
				continue;
			}
			const QString aname = chOps->attrName(ch, 0);
			AttrHandle h = attrOps->channelAttr(ch, aname);
			QVERIFY(h.ptr != nullptr);
			Result<QByteArray> r = attrOps->read(h);
			if(!r) {
				QVERIFY(r.error().code != 0 || !r.error().message.isEmpty());
			}
			attrOps->releaseAttr(h);
			readAny = true;
		}
	}

	ctxOps->destroyContext(ctx);
	if(!readAny) {
		QSKIP("no channel attributes at test URI");
	}
}

void TestAttrOps::rangeAndAvailableParse()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IChannelOps *chOps = b->channelOps();
	IAttrOps *attrOps = b->attrOps();
	DC_REQUIRE_CONTEXT();

	// getRange/getAvailable are the top v0/v1 divergence risk: v1 wraps the native
	// iio_attr_get_range/available, v0 parses the "*_available" attr string by hand.
	// For any channel attr whose name ends in "_available", both parsers must either
	// succeed with a coherent result or fail with a typed Error — never crash, and
	// never return success with an empty/degenerate payload.
	bool exercised = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		const unsigned int chCount = devOps->channelsCount(dev);
		for(unsigned int c = 0; c < chCount; ++c) {
			ChannelHandle ch = devOps->getChannel(dev, c);
			const unsigned int an = chOps->attrsCount(ch);
			for(unsigned int a = 0; a < an; ++a) {
				const QString aname = chOps->attrName(ch, a);
				if(!aname.endsWith(QStringLiteral("_available"))) {
					continue;
				}
				AttrHandle h = attrOps->channelAttr(ch, aname);
				QVERIFY(h.ptr != nullptr);

				QStringList values;
				Result<void> ra = attrOps->getAvailable(h, values);
				if(ra) {
					// A parsed "available" list must not be empty on success.
					QVERIFY(!values.isEmpty());
				} else {
					QVERIFY(ra.error().code != 0 || !ra.error().message.isEmpty());
				}

				double mn = 0, st = 0, mx = 0;
				Result<void> rr = attrOps->getRange(h, mn, st, mx);
				if(rr) {
					// A range only makes sense when max >= min.
					QVERIFY(mx >= mn);
				} else {
					QVERIFY(rr.error().code != 0 || !rr.error().message.isEmpty());
				}

				attrOps->releaseAttr(h);
				exercised = true;
			}
		}
	}

	ctxOps->destroyContext(ctx);
	if(!exercised) {
		QSKIP("no *_available channel attribute at test URI");
	}
}

void TestAttrOps::attrWrite()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IDeviceOps *devOps = b->deviceOps();
	IAttrOps *attrOps = b->attrOps();
	DC_REQUIRE_CONTEXT();
	DC_REQUIRE_MUTATION();

	// Writing an attribute changes device state — guarded. Read the current value,
	// write it straight back (a no-op change), and require the write to report a
	// typed Result. We only touch the first device attr to keep the footprint tiny.
	bool exercised = false;
	const unsigned int devCount = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < devCount && !exercised; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		if(devOps->attrsCount(dev) == 0) {
			continue;
		}
		const QString aname = devOps->attrName(dev, 0);
		AttrHandle h = attrOps->deviceAttr(dev, aname);
		QVERIFY(h.ptr != nullptr);
		Result<QByteArray> cur = attrOps->read(h);
		if(cur) {
			Result<void> w = attrOps->write(h, QString::fromUtf8(cur.value()).trimmed());
			if(!w) {
				QVERIFY(w.error().code != 0 || !w.error().message.isEmpty());
			}
			exercised = true;
		}
		attrOps->releaseAttr(h);
	}

	ctxOps->destroyContext(ctx);
	if(!exercised) {
		QSKIP("no writable device attribute at test URI");
	}
}

QTEST_MAIN(TestAttrOps)
#include "tst_attrops.moc"
