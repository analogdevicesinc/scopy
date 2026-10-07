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
 */

#include "m2kattributereader.h"
#include "m2kattributewriter.h"

#include "component/attribute.h"
#include "core/pooledcmdexecutor.h"

#include <QSignalSpy>
#include <QTest>
#include <qcoro/qcorofuture.h>

#include <stdexcept>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::adalm2000;

class TST_M2kAttribute : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void readPopulatesCachedValue();
	void readFailureEmitsReadFailed();
	void writeThenReadBackSettles();
	void writeFailureDoesNotSettle();
	void writeIsGivenTheValue();
	void readAsyncCarriesCommandId();
};

void TST_M2kAttribute::readPopulatesCachedValue()
{
	PooledCmdExecutor exec(1);
	Attribute attr;
	attr.setName("range");
	attr.addReadCapability(new M2kAttributeReader(
		nullptr, []() { return QByteArray("+/-25V"); }, &exec));

	QSignalSpy spy(&attr, &Attribute::valueChanged);
	attr.readCapability()->readAsync();
	QVERIFY(spy.wait());
	QCOMPARE(attr.cachedValue(), QStringLiteral("+/-25V"));
}

void TST_M2kAttribute::readFailureEmitsReadFailed()
{
	PooledCmdExecutor exec(1);
	M2kAttributeReader reader(
		nullptr, []() -> QByteArray { throw std::runtime_error("no device"); }, &exec);

	QSignalSpy failed(&reader, &AttributeReader::readFailed);
	const auto resp = QCoro::waitFor(reader.readAsync());
	QVERIFY(!bool(resp));
	QCOMPARE(failed.count(), 1);
}

void TST_M2kAttribute::writeThenReadBackSettles()
{
	PooledCmdExecutor exec(1);
	QByteArray held("+/-25V");

	Attribute attr;
	attr.setName("range");
	attr.addReadCapability(new M2kAttributeReader(
		nullptr, [&held]() { return held; }, &exec));
	attr.addWriteCapability(new M2kAttributeWriter(
		nullptr, [](const QString &) {}, &exec));

	attr.writeCapability()->writeAsync("+/-2.5V");
	QTRY_COMPARE(attr.cachedValue(), QStringLiteral("+/-25V"));
}

void TST_M2kAttribute::writeFailureDoesNotSettle()
{
	PooledCmdExecutor exec(1);
	int reads = 0;

	Attribute attr;
	attr.setName("range");
	attr.addReadCapability(new M2kAttributeReader(
		nullptr,
		[&reads]() {
			++reads;
			return QByteArray("+/-25V");
		},
		&exec));
	attr.addWriteCapability(new M2kAttributeWriter(
		nullptr, [](const QString &) { throw std::runtime_error("write refused"); }, &exec));

	QSignalSpy failed(attr.writeCapability(), &AttributeWriter::writeFailed);
	const auto resp = QCoro::waitFor(attr.writeCapability()->writeAsync("+/-2.5V"));
	QVERIFY(!bool(resp));
	QCOMPARE(failed.count(), 1);
	QCOMPARE(reads, 0);
}

void TST_M2kAttribute::writeIsGivenTheValue()
{
	PooledCmdExecutor exec(1);
	QString seen;
	M2kAttributeWriter writer(
		nullptr, [&seen](const QString &v) { seen = v; }, &exec);
	QVERIFY(bool(QCoro::waitFor(writer.writeAsync("+/-2.5V"))));
	QCOMPARE(seen, QStringLiteral("+/-2.5V"));
}

void TST_M2kAttribute::readAsyncCarriesCommandId()
{
	PooledCmdExecutor exec(1);
	M2kAttributeReader reader(
		nullptr, []() { return QByteArray("1"); }, &exec);
	const auto resp = QCoro::waitFor(reader.readAsync());
	QVERIFY(!resp.commandId().isNull());
}

QTEST_MAIN(TST_M2kAttribute)

#include "tst_m2kattribute.moc"
