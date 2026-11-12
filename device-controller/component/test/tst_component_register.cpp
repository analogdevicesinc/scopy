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

#include "component/backends/iio/iioregisterreader.h"
#include "component/backends/iio/iioregisterwriter.h"
#include "component/backends/iio/regmap/iioregisterreadstrategy.h"
#include "component/backends/iio/regmap/iioregisterwritestrategy.h"
#include "component/device.h"
#include "component/regmap/registerstrategies.h"
#include "core/pooledcmdexecutor.h"
#include "iioutil/ideviceops.h"

#include <QHash>
#include <QSignalSpy>
#include <QTest>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::iio;

namespace {

// In-memory IDeviceOps: only register I/O is exercised. addr 0xdead is poisoned
// to exercise the failure path.
class FakeDeviceOps : public scopy::iio::IDeviceOps
{
public:
	QHash<uint32_t, uint32_t> regs;
	QList<uint32_t> calls; // every address sent to the device

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
		calls.append(addr);
		if(addr == 0xdead) {
			return scopy::Unexpected{scopy::Error{-EIO, QStringLiteral("bad addr")}};
		}
		return regs.value(addr, 0);
	}
	scopy::Result<void> regWrite(scopy::iio::DeviceHandle, uint32_t addr, uint32_t val) override
	{
		calls.append(addr);
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
	void strategyReadRangeCarriesOn();
	void strategyWriteRangeCarriesOn();
	void strategyAddressSpace();
	void strategiesPerAddressSpace();
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

void TstComponentRegister::strategyReadRangeCarriesOn()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	ops.regs.insert(0x1, 0x11);
	ops.regs.insert(0x5, 0x55);
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};
	IIORegisterReader reader(&ops, dh, &exec);
	IIORegisterReadStrategy strategy(&reader, 0);

	// not contiguous, with a failing register in the middle
	const QVector<RegisterData> results =
		QCoro::waitFor(strategy.readRange({RegisterData{0x5}, RegisterData{0xdead}, RegisterData{0x1}}));
	QCOMPARE(results.size(), 3);
	QCOMPARE(results[0].address, 0x5u);
	QVERIFY(results[0].isOk());
	QCOMPARE(*results[0].value, 0x55u);
	QCOMPARE(results[1].address, 0xdeadu);
	QVERIFY(results[1].isError());
	QVERIFY(!results[1].value);
	QCOMPARE(results[1].error->code, -EIO);
	QCOMPARE(results[2].address, 0x1u);
	QVERIFY(results[2].isOk());
	QCOMPARE(*results[2].value, 0x11u);
	QVERIFY(results[2].completedAt.isValid());
}

void TstComponentRegister::strategyWriteRangeCarriesOn()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};
	IIORegisterWriter writer(&ops, dh, &exec);
	IIORegisterWriteStrategy strategy(&writer, 0);

	const QVector<RegisterData> results = QCoro::waitFor(strategy.writeRange(
		{RegisterData{0x1, 0xa}, RegisterData{0xdead, 0xb}, RegisterData{0x3, 0xc}, RegisterData{0x4}}));
	QCOMPARE(results.size(), 4);
	QVERIFY(results[0].isOk());
	QCOMPARE(*results[0].value, 0xau);
	QVERIFY(results[1].isError());
	// the write after the failed one is still attempted
	QVERIFY(results[2].isOk());
	QCOMPARE(ops.regs.value(0x3), 0xcu);
	// no value to write
	QVERIFY(results[3].isError());
	QCOMPARE(results[3].error->code, -EINVAL);
	QCOMPARE(ops.calls, (QList<uint32_t>{0x1, 0xdead, 0x3}));
}

void TstComponentRegister::strategyAddressSpace()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	ops.regs.insert(0x80000004, 0x44);
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};
	IIORegisterReader reader(&ops, dh, &exec);
	IIORegisterWriter writer(&ops, dh, &exec);
	IIORegisterReadStrategy read(&reader, 0x80000000);
	IIORegisterWriteStrategy write(&writer, 0x80000000);

	// the space goes to the device only, the results keep the template address
	const RegisterData r = QCoro::waitFor(read.read(RegisterData{0x4}));
	QCOMPARE(r.address, 0x4u);
	QCOMPARE(*r.value, 0x44u);
	const RegisterData w = QCoro::waitFor(write.write(RegisterData{0x8, 0x1}));
	QCOMPARE(w.address, 0x8u);
	QVERIFY(w.isOk());
	QCOMPARE(ops.calls, (QList<uint32_t>{0x80000004, 0x80000008}));
}

void TstComponentRegister::strategiesPerAddressSpace()
{
	PooledCmdExecutor exec(1);
	FakeDeviceOps ops;
	scopy::iio::DeviceHandle dh{reinterpret_cast<void *>(0x1)};
	scopy::component::Device device;
	new IIORegisterReader(&ops, dh, &exec, &device);
	new IIORegisterWriter(&ops, dh, &exec, &device);

	const RegisterStrategies plain = registerStrategies(&device, 0);
	QVERIFY(plain.read && plain.write);
	// asked again: the same pair
	const RegisterStrategies again = registerStrategies(&device, 0);
	QCOMPARE(again.read.data(), plain.read.data());
	QCOMPARE(again.write.data(), plain.write.data());
	// another address space: its own pair, owned by the device
	const RegisterStrategies axi = registerStrategies(&device, 0x80000000);
	QVERIFY(axi.read && axi.read != plain.read);
	QCOMPARE(axi.read->parent(), &device);
	QCOMPARE(device.findChildren<IRegisterReadStrategy *>().size(), 2);

	// a device with no register access has none
	scopy::component::Device noRegs;
	const RegisterStrategies none = registerStrategies(&noRegs, 0);
	QVERIFY(!none.read && !none.write);
}

QTEST_MAIN(TstComponentRegister)
#include "tst_component_register.moc"
