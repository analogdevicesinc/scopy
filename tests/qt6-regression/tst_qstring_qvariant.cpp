/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include <QTest>
#include <QString>
#include <QVariant>
#include <QRegularExpression>
#include <QList>
#include <QMap>
#include <QWidget>

class TST_QStringQVariant : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void qstring_number_int();
	void qstring_number_double();
	void qstring_arg_int();
	void qstring_arg_double();
	void qstring_arg_multiple();
	void qstring_contains_regex();
	void qvariant_typeId();
	void qvariant_type_comparison();
	void qvariant_toString();
	void std_as_const_list();
	void std_as_const_map();
	void setContentsMargins_equivalent();
};

void TST_QStringQVariant::qstring_number_int()
{
	QCOMPARE(QString::number(0), QString("0"));
	QCOMPARE(QString::number(42), QString("42"));
	QCOMPARE(QString::number(-1), QString("-1"));
	QCOMPARE(QString::number(2147483647), QString("2147483647"));
}

void TST_QStringQVariant::qstring_number_double()
{
	QCOMPARE(QString::number(3.14), QString("3.14"));
	QCOMPARE(QString::number(0.0), QString("0"));
	QCOMPARE(QString::number(1.0), QString("1"));
	QCOMPARE(QString::number(-2.5), QString("-2.5"));

	QString result = QString::number(1.23456789012345, 'g', 6);
	QCOMPARE(result, QString("1.23457"));
}

void TST_QStringQVariant::qstring_arg_int()
{
	QCOMPARE(QString("%1").arg(42), QString("42"));
	QCOMPARE(QString("%1").arg(0), QString("0"));
	QCOMPARE(QString("%1").arg(-1), QString("-1"));
}

void TST_QStringQVariant::qstring_arg_double()
{
	QCOMPARE(QString("%1").arg(3.14), QString("3.14"));
	QCOMPARE(QString("%1").arg(0.0), QString("0"));
}

void TST_QStringQVariant::qstring_arg_multiple()
{
	QString result = QString("%1/%2/%3").arg("a").arg("b").arg("c");
	QCOMPARE(result, QString("a/b/c"));

	result = QString("%1 = %2").arg("key").arg(42);
	QCOMPARE(result, QString("key = 42"));

	result = QString("(%1, %2, %3)").arg(1).arg(2).arg(3);
	QCOMPARE(result, QString("(1, 2, 3)"));
}

void TST_QStringQVariant::qstring_contains_regex()
{
	QRegularExpression ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-"
				   "9][0-9]|2[0-4][0-9]|25[0-5])$");

	QString uri = "192.168.2.1";
	QVERIFY(uri.contains(ipRegex));

	uri = "usb:1.2.3";
	QVERIFY(!uri.contains(ipRegex));
}

void TST_QStringQVariant::qvariant_typeId()
{
	QVariant intVar(42);
	QCOMPARE(intVar.typeId(), QMetaType::Int);

	QVariant strVar(QString("hello"));
	QCOMPARE(strVar.typeId(), QMetaType::QString);

	QVariant dblVar(3.14);
	QCOMPARE(dblVar.typeId(), QMetaType::Double);

	QVariant boolVar(true);
	QCOMPARE(boolVar.typeId(), QMetaType::Bool);

	QVariant invalidVar;
	QVERIFY(!invalidVar.isValid());
}

void TST_QStringQVariant::qvariant_type_comparison()
{
	QVariant v1(42);
	QVariant v2(QString("42"));

	QVERIFY(v1.typeId() != v2.typeId());
	QCOMPARE(v1.typeId(), QMetaType::Int);
	QCOMPARE(v2.typeId(), QMetaType::QString);

	QCOMPARE(v1.toString(), v2.toString());
}

void TST_QStringQVariant::qvariant_toString()
{
	QCOMPARE(QVariant(42).toString(), QString("42"));
	QCOMPARE(QVariant(3.14).toString(), QString("3.14"));
	QCOMPARE(QVariant(true).toString(), QString("true"));
	QCOMPARE(QVariant(false).toString(), QString("false"));
	QCOMPARE(QVariant(QString("test")).toString(), QString("test"));
}

void TST_QStringQVariant::std_as_const_list()
{
	QList<int> list = {1, 2, 3, 4, 5};
	QList<int> copy = list;

	int sum = 0;
	for(const auto &val : std::as_const(list)) {
		sum += val;
	}
	QCOMPARE(sum, 15);

	QCOMPARE(list, copy);
}

void TST_QStringQVariant::std_as_const_map()
{
	QMap<QString, int> map;
	map["a"] = 1;
	map["b"] = 2;
	map["c"] = 3;
	QMap<QString, int> copy = map;

	int sum = 0;
	for(auto it = std::as_const(map).begin(); it != std::as_const(map).end(); ++it) {
		sum += it.value();
	}
	QCOMPARE(sum, 6);

	QCOMPARE(map, copy);
}

void TST_QStringQVariant::setContentsMargins_equivalent()
{
	QWidget w;
	w.setContentsMargins(0, 0, 0, 0);

	auto margins = w.contentsMargins();
	QCOMPARE(margins.left(), 0);
	QCOMPARE(margins.top(), 0);
	QCOMPARE(margins.right(), 0);
	QCOMPARE(margins.bottom(), 0);

	w.setContentsMargins(5, 10, 15, 20);
	margins = w.contentsMargins();
	QCOMPARE(margins.left(), 5);
	QCOMPARE(margins.top(), 10);
	QCOMPARE(margins.right(), 15);
	QCOMPARE(margins.bottom(), 20);
}

QTEST_MAIN(TST_QStringQVariant)
#include "tst_qstring_qvariant.moc"
