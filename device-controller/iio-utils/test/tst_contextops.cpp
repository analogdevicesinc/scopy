#include "backendfixture.h"

#include "iioutil/icontextops.h"
#include "iioutil/iattrops.h"

using namespace scopy;
using namespace scopy::iio;

class TestContextOps : public QObject
{
	Q_OBJECT
private slots:
	void pingAlive();
	void pingAlive_data() { dctest::addBackendRows(); }

	void versionAndName();
	void versionAndName_data() { dctest::addBackendRows(); }

	void attrEnumerationConsistent();
	void attrEnumerationConsistent_data() { dctest::addBackendRows(); }

	void attrReadConsistent();
	void attrReadConsistent_data() { dctest::addBackendRows(); }

	void findDeviceRoundTrips();
	void findDeviceRoundTrips_data() { dctest::addBackendRows(); }
};

void TestContextOps::pingAlive()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ops = b->contextOps();
    DC_REQUIRE_CONTEXT();

	QVERIFY(ops->ping(ctx));
	ops->destroyContext(ctx);
}

void TestContextOps::versionAndName()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ops = b->contextOps();
	DC_REQUIRE_CONTEXT();

	// Metadata getters must be callable without crashing.
	const QString name = ops->name(ctx);
	Q_UNUSED(name);
	// description is free-form and may be empty, but must be callable.
	const QString desc = ops->description(ctx);
	Q_UNUSED(desc);
	unsigned int major = 0, minor = 0;
	QString tag;
	ops->version(ctx, major, minor, tag);

	ops->destroyContext(ctx);
}

void TestContextOps::attrEnumerationConsistent()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ops = b->contextOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int n = ops->attrsCount(ctx);
	for(unsigned int i = 0; i < n; ++i) {
		QVERIFY(!ops->attrName(ctx, i).isEmpty());
	}

	ops->destroyContext(ctx);
}

void TestContextOps::attrReadConsistent()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	IAttrOps *attrOps = b->attrOps();
	DC_REQUIRE_CONTEXT();

	// Every enumerated context attribute must resolve to a handle and read back
	// as either a value or a typed Error — never crash. This is a read-only path.
	const unsigned int n = ctxOps->attrsCount(ctx);
	for(unsigned int i = 0; i < n; ++i) {
		const QString aname = ctxOps->attrName(ctx, i);
		AttrHandle h = attrOps->contextAttr(ctx, aname);
		QVERIFY(h.ptr != nullptr);
		Result<QByteArray> r = attrOps->read(h);
		if(!r) {
			QVERIFY(r.error().code != 0 || !r.error().message.isEmpty());
		}
		attrOps->releaseAttr(h);
	}

	ctxOps->destroyContext(ctx);
	if(n == 0) {
		QSKIP("no context attributes at test URI");
	}
}

void TestContextOps::findDeviceRoundTrips()
{
	DC_REQUIRE_BACKEND();
	IContextOps *ctxOps = b->contextOps();
	DC_REQUIRE_CONTEXT();

	const unsigned int count = ctxOps->devicesCount(ctx);
	for(unsigned int i = 0; i < count; ++i) {
		DeviceHandle dev = ctxOps->getDevice(ctx, i);
		QVERIFY(dev.ptr != nullptr);
	}
	// findDevice on a name that cannot exist must return a null handle, not crash.
	DeviceHandle missing = ctxOps->findDevice(ctx, QStringLiteral("__no_such_device__"));
	QVERIFY(missing.ptr == nullptr);

	ctxOps->destroyContext(ctx);
}

QTEST_MAIN(TestContextOps)
#include "tst_contextops.moc"
