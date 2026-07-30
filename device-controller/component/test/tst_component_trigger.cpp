#include "component/backends/iio/iiotrigger.h"
#include "component/backends/iio/iiotriggerable.h"
#include "core/pooledcmdexecutor.h"
#include "iioutil/ideviceops.h"

#include <QSignalSpy>
#include <QTest>

using namespace scopy;
using namespace scopy::component::iio;

namespace {

// In-memory IDeviceOps: DeviceHandle.ptr is a FakeDev*. Only trigger relations
// and name/isTrigger are exercised.
struct FakeDev
{
	QString name;
	bool isTrigger = false;
	FakeDev *trigger = nullptr;
};

class FakeDeviceOps : public scopy::iio::IDeviceOps
{
public:
	static FakeDev *dev(scopy::iio::DeviceHandle h) { return static_cast<FakeDev *>(h.ptr); }

	QString id(scopy::iio::DeviceHandle) const override { return {}; }
	QString name(scopy::iio::DeviceHandle h) const override { return dev(h)->name; }
	QString label(scopy::iio::DeviceHandle) const override { return {}; }
	bool isTrigger(scopy::iio::DeviceHandle h) const override { return dev(h)->isTrigger; }
	bool isHwmon(scopy::iio::DeviceHandle) const override { return false; }
	unsigned int channelsCount(scopy::iio::DeviceHandle) const override { return 0; }
	scopy::iio::ChannelHandle getChannel(scopy::iio::DeviceHandle, unsigned int) const override { return {}; }
	scopy::iio::ChannelHandle findChannel(scopy::iio::DeviceHandle, const QString &, bool) const override
	{
		return {};
	}
	unsigned int attrsCount(scopy::iio::DeviceHandle) const override { return 0; }
	QString attrName(scopy::iio::DeviceHandle, unsigned int) const override { return {}; }
	unsigned int debugAttrsCount(scopy::iio::DeviceHandle) const override { return 0; }
	QString debugAttrName(scopy::iio::DeviceHandle, unsigned int) const override { return {}; }
	unsigned int buffersCount(scopy::iio::DeviceHandle) const override { return 1; }
	unsigned int bufferAttrsCount(scopy::iio::DeviceHandle, unsigned int) const override { return 0; }
	QString bufferAttrName(scopy::iio::DeviceHandle, unsigned int, unsigned int) const override { return {}; }
	scopy::iio::DeviceHandle getTrigger(scopy::iio::DeviceHandle h) const override { return {dev(h)->trigger}; }
	ssize_t sampleSize(scopy::iio::DeviceHandle, scopy::iio::ChannelsMaskHandle) const override { return 0; }

	scopy::Result<uint32_t> regRead(scopy::iio::DeviceHandle, uint32_t) override { return 0u; }
	scopy::Result<void> regWrite(scopy::iio::DeviceHandle, uint32_t, uint32_t) override { return {}; }
	scopy::Result<void> setTrigger(scopy::iio::DeviceHandle h, scopy::iio::DeviceHandle trig) override
	{
		dev(h)->trigger = trig.ptr ? dev(trig) : nullptr;
		return {};
	}
};

} // namespace

class TstComponentTrigger : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void setAndClear();
};

void TstComponentTrigger::setAndClear()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	FakeDev trigDev{QStringLiteral("trigger0"), true, nullptr};
	FakeDev adcDev{QStringLiteral("adc0"), false, nullptr};

	IIOTrigger source(&ops, {&trigDev});
	IIOTriggerable triggerable(&ops, {&adcDev}, &exec);

	QCOMPARE(source.name(), QStringLiteral("trigger0"));
	QVERIFY(!triggerable.hasTrigger());

	QSignalSpy ok(&triggerable, &IIOTriggerable::triggerSucceeded);
	QVERIFY(triggerable.setTrigger(&source));
	QCOMPARE(ok.count(), 1);
	QVERIFY(triggerable.hasTrigger());
	QCOMPARE(triggerable.assignedTriggerName(), QStringLiteral("trigger0"));

	QVERIFY(triggerable.clearTrigger());
	QCOMPARE(ok.count(), 2);
	QVERIFY(!triggerable.hasTrigger());
}

QTEST_MAIN(TstComponentTrigger)
#include "tst_component_trigger.moc"
