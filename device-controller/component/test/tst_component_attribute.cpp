#include "fakeattrops.h"

#include "component/attribute.h"
#include "component/backends/iio/iioattributereader.h"
#include "component/backends/iio/iioattributewriter.h"
#include "component/tags.h"

#include "core/pooledcmdexecutor.h"

#include <QSignalSpy>
#include <QTest>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component;

class TstComponentAttribute : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void readPopulatesCachedValue();
	void writeThenReadBackSettles();
	void readOnlyHasNoWriter();
	void tagsRoundTrip();
	void readAsyncCarriesCommandId();
};

void TstComponentAttribute::readPopulatesCachedValue()
{
	dctest::FakeAttrOps ops;
	auto handle = ops.make("frequency", "100");
	PooledCmdExecutor exec(1);

	Attribute attr;
	attr.setName("frequency");
	attr.addReadCapability(new component::iio::IIOAttributeReader(&ops, handle, &exec));

	QSignalSpy spy(&attr, &Attribute::valueChanged);
	attr.readCapability()->readAsync();
	QVERIFY(spy.wait());
	QCOMPARE(attr.cachedValue(), QStringLiteral("100"));
}

void TstComponentAttribute::writeThenReadBackSettles()
{
	dctest::FakeAttrOps ops;
	auto handle = ops.make("frequency", "100");
	PooledCmdExecutor exec(1);

	Attribute attr;
	attr.setName("frequency");
	attr.addReadCapability(new component::iio::IIOAttributeReader(&ops, handle, &exec));
	attr.addWriteCapability(new component::iio::IIOAttributeWriter(&ops, handle, &exec));

	QSignalSpy spy(&attr, &Attribute::valueChanged);
	attr.writeCapability()->writeAsync("250");

	// write → writeSucceeded → readAsync → readSucceeded → cachedValue == "250"
	QTRY_COMPARE(attr.cachedValue(), QStringLiteral("250"));
	QVERIFY(spy.count() >= 1);
}

void TstComponentAttribute::readOnlyHasNoWriter()
{
	dctest::FakeAttrOps ops;
	auto handle = ops.make("name", "ad9361");
	PooledCmdExecutor exec(1);

	Attribute attr;
	attr.setName("name");
	attr.addReadCapability(new component::iio::IIOAttributeReader(&ops, handle, &exec));

	QVERIFY(!attr.isWritable());
	QCOMPARE(attr.writeCapability(), nullptr);
}

void TstComponentAttribute::tagsRoundTrip()
{
	QObject root;
	auto *attr = new Attribute(&root);
	attr->setName("sampling_frequency");
	QVERIFY(!hasTag(attr, tags::SampleRate));
	addTag(attr, tags::SampleRate);
	QVERIFY(hasTag(attr, tags::SampleRate));

	QCOMPARE(findByTag<Attribute>(&root, tags::SampleRate).size(), 1);
}

void TstComponentAttribute::readAsyncCarriesCommandId()
{
	dctest::FakeAttrOps ops;
	auto handle = ops.make("frequency", "1");
	PooledCmdExecutor exec(1);

	component::iio::IIOAttributeReader reader(&ops, handle, &exec);
	const auto resp = QCoro::waitFor(reader.readAsync());
	QVERIFY(!resp.commandId().isNull());
}

QTEST_MAIN(TstComponentAttribute)
#include "tst_component_attribute.moc"
