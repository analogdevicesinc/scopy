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

#include "component/backends/file/regmap/fileregisterreadstrategy.h"
#include "component/backends/file/regmap/fileregisterwritestrategy.h"
#include "component/regmap/registerstrategies.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <qcoro/qcorotask.h>

#include <cerrno>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::component::file;

namespace {
void writeText(const QString &path, const QString &text)
{
	QFile file(path);
	QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
	file.write(text.toUtf8());
}

QString readText(const QString &path)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return {};
	}
	return QString::fromUtf8(file.readAll());
}
} // namespace

class TstComponentFileRegister : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void readRangeOnePass();
	void writeRangeInOrder();
	void missingFile();
	void strategiesPerPath();

private:
	QTemporaryDir m_dir;
};

void TstComponentFileRegister::readRangeOnePass()
{
	const QString path = m_dir.filePath("read.csv");
	writeText(path, "# comment\n0x01,0x0A\n5,ff\n0x01,0x0B\n");
	FileRegisterReadStrategy strategy(path);

	const QVector<RegisterData> results =
		QCoro::waitFor(strategy.readRange({RegisterData{0x5}, RegisterData{0x7}, RegisterData{0x1}}));
	QCOMPARE(results.size(), 3);
	QCOMPARE(results[0].address, 0x5u);
	QCOMPARE(*results[0].value, 0xffu);
	QCOMPARE(results[1].address, 0x7u);
	QVERIFY(results[1].isError());
	QCOMPARE(results[1].error->code, -ENOENT);
	// the first line of an address wins
	QCOMPARE(*results[2].value, 0xau);
	// one pass, one stamp
	QCOMPARE(results[0].completedAt, results[1].completedAt);
	QCOMPARE(results[1].completedAt, results[2].completedAt);
}

void TstComponentFileRegister::writeRangeInOrder()
{
	const QString path = m_dir.filePath("write.csv");
	writeText(path, "0x001,0x00000001\n");
	FileRegisterWriteStrategy strategy(path);

	const QVector<RegisterData> results = QCoro::waitFor(
		strategy.writeRange({RegisterData{0x1, 0x2}, RegisterData{0x9, 0x3}, RegisterData{0x1, 0x4}}));
	QCOMPARE(results.size(), 3);
	for(const RegisterData &r : results) {
		QVERIFY(r.isOk());
		QCOMPARE(r.completedAt, results[0].completedAt);
	}
	// the digit widths of the replaced line stay, the last write of an address wins
	QCOMPARE(readText(path), QString("0x001,0x00000004\n0x009,0x03\n"));
}

void TstComponentFileRegister::missingFile()
{
	const QString path = m_dir.filePath("missing.csv");
	FileRegisterReadStrategy read(path);
	FileRegisterWriteStrategy write(path);

	const QVector<RegisterData> r = QCoro::waitFor(read.readRange({RegisterData{0x1}, RegisterData{0x2}}));
	QCOMPARE(r.size(), 2);
	QVERIFY(r[0].isError() && r[1].isError());
	QCOMPARE(r[0].error->code, -EIO);
	const RegisterData w = QCoro::waitFor(write.write(RegisterData{0x1, 0x1}));
	QVERIFY(w.isError());
	QCOMPARE(w.error->code, -EIO);
}

void TstComponentFileRegister::strategiesPerPath()
{
	const QString a = m_dir.filePath("a.csv");
	const QString b = m_dir.filePath("b.csv");
	const RegisterStrategies first = fileRegisterStrategies(a);
	QVERIFY(first.read && first.write);
	const RegisterStrategies again = fileRegisterStrategies(a);
	QCOMPARE(again.read.data(), first.read.data());
	QCOMPARE(again.write.data(), first.write.data());
	const RegisterStrategies other = fileRegisterStrategies(b);
	QVERIFY(other.read && other.read != first.read);
	QVERIFY(!fileRegisterStrategies(QString()).read);
}

QTEST_MAIN(TstComponentFileRegister)
#include "tst_component_fileregister.moc"
