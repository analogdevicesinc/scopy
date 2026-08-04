#include "component/backends/iio/iioregisterreader.h"
#include "component/backends/iio/iioregisterwriter.h"
#include "core/pooledcmdexecutor.h"
#include "iioutil/ideviceops.h"

#include <QHash>
#include <QSignalSpy>
#include <QTest>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

namespace {

// In-memory IDeviceOps: only register I/O is exercised. addr 0xdead is poisoned
// to exercise the failure path.
class FakeDeviceOps : public scopy::iio::IDeviceOps
{
public:
	QHash<uint32_t, uint32_t> regs;

	QString id(scopy::iio::DeviceHandle) const override { return {}; }
	QString name(scopy::iio::DeviceHandle) const override { return {}; }
	QString label(scopy::iio::DeviceHandle) const override { return {}; }
	bool isTrigger(scopy::iio::DeviceHandle) const override { return false; }
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
	scopy::iio::DeviceHandle getTrigger(scopy::iio::DeviceHandle) const override { return {}; }
	ssize_t sampleSize(scopy::iio::DeviceHandle, scopy::iio::ChannelsMaskHandle) const override { return 0; }

	scopy::Result<uint32_t> regRead(scopy::iio::DeviceHandle, uint32_t addr) override
	{
		if(addr == 0xdead) {
			return scopy::Unexpected{scopy::Error{-EIO, QStringLiteral("bad addr")}};
		}
		return regs.value(addr, 0);
	}
	scopy::Result<void> regWrite(scopy::iio::DeviceHandle, uint32_t addr, uint32_t val) override
	{
		if(addr == 0xdead) {
			return scopy::Unexpected{scopy::Error{-EIO, QStringLiteral("bad addr")}};
		}
		regs.insert(addr, val);
		return {};
	}
	scopy::Result<void> setTrigger(scopy::iio::DeviceHandle, scopy::iio::DeviceHandle) override { return {}; }
};

} // namespace

class TstComponentRegister : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void writeThenReadBack();
	void readEmitsSucceeded();
	void failurePaths();
};

void TstComponentRegister::writeThenReadBack()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};

	IIORegisterWriter writer(&ops, dh, &exec);
	IIORegisterReader reader(&ops, dh, &exec);

	QVERIFY(QCoro::waitFor(writer.writeAsync(0x10, 0xabcd)));
	const auto r = QCoro::waitFor(reader.readAsync(0x10));
	QVERIFY(r);
	QCOMPARE(r.value(), 0xabcdu);
}

void TstComponentRegister::readEmitsSucceeded()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	ops.regs.insert(0x20, 0x55);
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};

	IIORegisterReader reader(&ops, dh, &exec);
	QSignalSpy ok(&reader, &IIORegisterReader::readSucceeded);
	QVERIFY(QCoro::waitFor(reader.readAsync(0x20)));
	QCOMPARE(ok.count(), 1);
	QCOMPARE(ok.at(0).at(0).toUInt(), 0x20u);
	QCOMPARE(ok.at(0).at(1).toUInt(), 0x55u);
}

void TstComponentRegister::failurePaths()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};

	IIORegisterReader reader(&ops, dh, &exec);
	IIORegisterWriter writer(&ops, dh, &exec);
	QSignalSpy rfail(&reader, &IIORegisterReader::readFailed);
	QSignalSpy wfail(&writer, &IIORegisterWriter::writeFailed);

	QVERIFY(!QCoro::waitFor(reader.readAsync(0xdead)));
	QVERIFY(!QCoro::waitFor(writer.writeAsync(0xdead, 1)));
	QCOMPARE(rfail.count(), 1);
	QCOMPARE(wfail.count(), 1);
}

QTEST_MAIN(TstComponentRegister)
#include "tst_component_register.moc"
