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

#include "core/result.h"

#include <QTest>

#include <cerrno>

using namespace scopy;

class TestResult : public QObject
{
	Q_OBJECT
private slots:
	void successValue();
	void errorValue();
	void valueOr();
	void voidSuccess();
	void voidError();
	void sameTypeTag();
	void errorString();
};

void TestResult::successValue()
{
	Result<QByteArray> r(QByteArray("12345"));
	QVERIFY(r.hasValue());
	QVERIFY(bool(r));
	QCOMPARE(QString::fromUtf8(r.value()), QStringLiteral("12345"));
}

void TestResult::errorValue()
{
	Result<QByteArray> r(Unexpected{Error{-ENOENT, QStringLiteral("missing")}});
	QVERIFY(!r.hasValue());
	QVERIFY(!bool(r));
	QCOMPARE(r.error().code, -ENOENT);
	QCOMPARE(r.error().errorCode(), ENOENT);
	QCOMPARE(r.error().message, QStringLiteral("missing"));
}

void TestResult::valueOr()
{
	Result<int> ok(42);
	QCOMPARE(ok.valueOr(-1), 42);

	Result<int> bad(Unexpected{Error{-EIO}});
	QCOMPARE(bad.valueOr(-1), -1);
}

void TestResult::voidSuccess()
{
	Result<void> r;
	QVERIFY(r.hasValue());
	QVERIFY(bool(r));
}

void TestResult::voidError()
{
	Result<void> r(Unexpected{Error{-EIO, QStringLiteral("io")}});
	QVERIFY(!r.hasValue());
	QVERIFY(!bool(r));
	QCOMPARE(r.error().code, -EIO);
}

void TestResult::sameTypeTag()
{
	// T == E must stay unambiguous thanks to the Unexpected<E> tag in the variant.
	Result<Error, Error> value(Error{7, QStringLiteral("payload")});
	QVERIFY(value.hasValue());
	QCOMPARE(value.value().code, 7);

	Result<Error, Error> err(Unexpected{Error{-1, QStringLiteral("fail")}});
	QVERIFY(!err.hasValue());
	QCOMPARE(err.error().code, -1);
}

void TestResult::errorString()
{
	// Explicit message is returned verbatim.
	Error withMsg{-ENOENT, QStringLiteral("custom")};
	QCOMPARE(withMsg.errorString(), QStringLiteral("custom"));

	// No message + negative code derives one from strerror_r.
	Error noMsg{-ENOENT, {}};
	QVERIFY(!noMsg.errorString().isEmpty());

	// Non-error code with no message yields an empty string.
	Error ok{0, {}};
	QVERIFY(ok.errorString().isEmpty());
}

QTEST_MAIN(TestResult)
#include "tst_result.moc"
