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
#include <QRegularExpression>

class TST_RegexPatterns : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	// JESD parser patterns (jesdstatusparser.cpp)
	void extractLaneNumber();
	void regexMatch_errors();
	void regexMatch_cgsState();
	void regexMatch_frameSynchronization();
	void regexMatchMultiple_latency();
	void regexMatch_laneAlignmentSequence();
	void regexMatchMultiple_ids();
	void regexMatchMultiple_frameControl();
	void regexMatchMultiple_checksum();
	void regexMatchMultiple_adjustment();
	void regexMatch_linkState();
	void regexMatch_measuredClock();
	void regexMatch_multiblockAlignment();

	// SpinboxA patterns (spinbox_a.cpp)
	void spinbox_baseRegex();
	void spinbox_withUnits();
	void spinbox_unicodeUnits();
	void spinbox_negativeLookahead();

	// IP validation patterns (iiotabwidget.cpp)
	void ipValidation_valid();
	void ipValidation_invalid();
	void ipValidation_edge();
};

// --- JESD parser patterns ---

void TST_RegexPatterns::extractLaneNumber()
{
	QRegularExpression regex("lane(\\d+)_info");

	QRegularExpressionMatch match = regex.match("lane0_info");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("0"));

	match = regex.match("lane7_info");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("7"));

	match = regex.match("lane15_info");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("15"));

	match = regex.match("not_a_lane");
	QVERIFY(!match.hasMatch());
}

void TST_RegexPatterns::regexMatch_errors()
{
	QRegularExpression regex("Errors: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("Errors: 0\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("0"));

	match = regex.match("Errors: 42 ");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("42"));

	match = regex.match("Errors: NONE");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("NONE"));
}

void TST_RegexPatterns::regexMatch_cgsState()
{
	QRegularExpression regex("CGS state: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("CGS state: DATA\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("DATA"));

	match = regex.match("CGS state: INIT ");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("INIT"));
}

void TST_RegexPatterns::regexMatch_frameSynchronization()
{
	QRegularExpression regex("Initial Frame Synchronization: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("Initial Frame Synchronization: YES\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("YES"));

	match = regex.match("Initial Frame Synchronization: NO ");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("NO"));
}

void TST_RegexPatterns::regexMatchMultiple_latency()
{
	QRegularExpression regex("Lane Latency: (\\S+) \\(min\\/max (\\S+)\\/(\\S+)\\)?[ \n]*$");

	QRegularExpressionMatch match = regex.match("Lane Latency: 15 (min/max 3/5)\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.lastCapturedIndex(), 3);
	QCOMPARE(match.captured(1), QString("15"));
	QCOMPARE(match.captured(2), QString("3"));
	QCOMPARE(match.captured(3), QString("5)"));

	match = regex.match("Lane Latency: 0 (min/max 0/0)");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("0"));
	QCOMPARE(match.captured(2), QString("0"));
	QCOMPARE(match.captured(3), QString("0)"));
}

void TST_RegexPatterns::regexMatch_laneAlignmentSequence()
{
	QRegularExpression regex("Initial Lane Alignment Sequence: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("Initial Lane Alignment Sequence: YES\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("YES"));
}

void TST_RegexPatterns::regexMatchMultiple_ids()
{
	QRegularExpression regex("DID: (\\S+), BID: (\\S+), LID: (\\S+), L: (\\S+), SCR: (\\S+), F: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("DID: 0, BID: 0, LID: 0, L: 4, SCR: 1, F: 1\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.lastCapturedIndex(), 6);
	QCOMPARE(match.captured(1), QString("0"));
	QCOMPARE(match.captured(2), QString("0"));
	QCOMPARE(match.captured(3), QString("0"));
	QCOMPARE(match.captured(4), QString("4"));
	QCOMPARE(match.captured(5), QString("1"));
	QCOMPARE(match.captured(6), QString("1"));

	match = regex.match("DID: 255, BID: 15, LID: 31, L: 8, SCR: 0, F: 4\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("255"));
	QCOMPARE(match.captured(4), QString("8"));
}

void TST_RegexPatterns::regexMatchMultiple_frameControl()
{
	QRegularExpression regex(
		"K: (\\S+), M: (\\S+), N: (\\S+), CS: (\\S+), N': (\\S+), S: (\\S+), HD: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("K: 32, M: 2, N: 14, CS: 0, N': 16, S: 1, HD: 1\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.lastCapturedIndex(), 7);
	QCOMPARE(match.captured(1), QString("32"));
	QCOMPARE(match.captured(2), QString("2"));
	QCOMPARE(match.captured(3), QString("14"));
	QCOMPARE(match.captured(4), QString("0"));
	QCOMPARE(match.captured(5), QString("16"));
	QCOMPARE(match.captured(6), QString("1"));
	QCOMPARE(match.captured(7), QString("1"));
}

void TST_RegexPatterns::regexMatchMultiple_checksum()
{
	QRegularExpression regex("FCHK: (\\S+), CF: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("FCHK: 0x1A, CF: 0\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.lastCapturedIndex(), 2);
	QCOMPARE(match.captured(1), QString("0x1A"));
	QCOMPARE(match.captured(2), QString("0"));
}

void TST_RegexPatterns::regexMatchMultiple_adjustment()
{
	QRegularExpression regex(
		"ADJCNT: (\\S+), PHADJ: (\\S+), ADJDIR: (\\S+), JESDV: (\\S+), SUBCLASS: (\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("ADJCNT: 0, PHADJ: 0, ADJDIR: 0, JESDV: 1, SUBCLASS: 1\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.lastCapturedIndex(), 5);
	QCOMPARE(match.captured(1), QString("0"));
	QCOMPARE(match.captured(2), QString("0"));
	QCOMPARE(match.captured(3), QString("0"));
	QCOMPARE(match.captured(4), QString("1"));
	QCOMPARE(match.captured(5), QString("1"));
}

void TST_RegexPatterns::regexMatch_linkState()
{
	QRegularExpression regex("Link is (\\S+)");

	QRegularExpressionMatch match = regex.match("Link is enabled");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("enabled"));

	match = regex.match("Link is disabled");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("disabled"));
}

void TST_RegexPatterns::regexMatch_measuredClock()
{
	QRegularExpression regex("Measured Link Clock: (\\S+) MHz");

	QRegularExpressionMatch match = regex.match("Measured Link Clock: 250.000 MHz");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("250.000"));

	match = regex.match("Measured Link Clock: 125.5 MHz");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("125.5"));
}

void TST_RegexPatterns::regexMatch_multiblockAlignment()
{
	QRegularExpression regex("State of Extended multiblock alignment:(\\S+)[ \\n]?");

	QRegularExpressionMatch match = regex.match("State of Extended multiblock alignment:LOCKED\n");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("LOCKED"));
}

// --- SpinboxA patterns ---

void TST_RegexPatterns::spinbox_baseRegex()
{
	QRegularExpression regex("^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))");

	QRegularExpressionMatch match = regex.match("123.456");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("123.456"));
	QCOMPARE(match.captured(2), QString(""));
	QCOMPARE(match.captured(3), QString("123"));
	QCOMPARE(match.captured(4), QString("."));
	QCOMPARE(match.captured(5), QString("456"));

	match = regex.match("-42");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("-42"));
	QCOMPARE(match.captured(2), QString("-"));
	QCOMPARE(match.captured(3), QString("4"));
	QCOMPARE(match.captured(5), QString("2"));

	match = regex.match("+0.001");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(2), QString("+"));
	QCOMPARE(match.captured(4), QString("."));
	QCOMPARE(match.captured(5), QString("001"));
}

void TST_RegexPatterns::spinbox_withUnits()
{
	QString regex = "^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))([MKmk]?)";
	QRegularExpression re(regex);

	QRegularExpressionMatch match = re.match("100M");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("100"));
	QCOMPARE(match.captured(6), QString("M"));

	match = re.match("3.14k");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("3.14"));
	QCOMPARE(match.captured(6), QString("k"));

	match = re.match("42");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(1), QString("42"));
	QCOMPARE(match.captured(6), QString(""));
}

void TST_RegexPatterns::spinbox_unicodeUnits()
{
	// After μ→u and π→r substitution, the character class includes u and r
	QString regex = "^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))([MKmkur]?)";
	QRegularExpression re(regex);

	QRegularExpressionMatch match = re.match("100u");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(6), QString("u"));

	match = re.match("3.14r");
	QVERIFY(match.hasMatch());
	QCOMPARE(match.captured(6), QString("r"));
}

void TST_RegexPatterns::spinbox_negativeLookahead()
{
	QRegularExpression regex("^(?!^.{18})(([+,-]?)([0-9]*)([.]?)([0-9]+))");

	// 17 chars should match
	QRegularExpressionMatch match = regex.match("12345678901234567");
	QVERIFY(match.hasMatch());

	// 18 chars should fail the negative lookahead
	match = regex.match("123456789012345678");
	QVERIFY(!match.hasMatch());
}

// --- IP validation patterns ---

void TST_RegexPatterns::ipValidation_valid()
{
	QRegularExpression ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-"
				   "9][0-9]|2[0-4][0-9]|25[0-5])$");

	QVERIFY(ipRegex.match("192.168.1.1").hasMatch());
	QVERIFY(ipRegex.match("10.0.0.1").hasMatch());
	QVERIFY(ipRegex.match("172.16.0.100").hasMatch());
	QVERIFY(ipRegex.match("8.8.8.8").hasMatch());
	QVERIFY(ipRegex.match("1.2.3.4").hasMatch());
}

void TST_RegexPatterns::ipValidation_invalid()
{
	QRegularExpression ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-"
				   "9][0-9]|2[0-4][0-9]|25[0-5])$");

	QVERIFY(!ipRegex.match("256.1.1.1").hasMatch());
	QVERIFY(!ipRegex.match("1.2.3.256").hasMatch());
	QVERIFY(!ipRegex.match("abc.def.ghi.jkl").hasMatch());
	QVERIFY(!ipRegex.match("").hasMatch());
	QVERIFY(!ipRegex.match("1.2.3").hasMatch());
	QVERIFY(!ipRegex.match("1.2.3.4.5").hasMatch());
}

void TST_RegexPatterns::ipValidation_edge()
{
	QRegularExpression ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-"
				   "9][0-9]|2[0-4][0-9]|25[0-5])$");

	QVERIFY(ipRegex.match("0.0.0.0").hasMatch());
	QVERIFY(ipRegex.match("255.255.255.255").hasMatch());
	QVERIFY(ipRegex.match("127.0.0.1").hasMatch());
	QVERIFY(ipRegex.match("249.249.249.249").hasMatch());
}

QTEST_MAIN(TST_RegexPatterns)
#include "tst_regex_patterns.moc"
