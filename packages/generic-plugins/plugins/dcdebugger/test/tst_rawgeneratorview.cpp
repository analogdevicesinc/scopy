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

#include "fakestream.h"
#include "generation/rawgeneratorview.h"
#include "generation/sampletable.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTemporaryDir>
#include <QtTest>

#include <menuspinbox.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotwidget.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// Raw generator: the buffer is a table of raw codes, edited, pasted or loaded from a file.
class TST_RawGeneratorView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void parseHeaderAndIndexColumn();
	void parseSeparators();
	void parseRejectsText();
	void tableEdits();
	void rowsFollowTheSpinbox();
	void loadMapsColumnsInOrder();
	void loadIgnoresExtraColumns();
	void pasteGrowsTheTable();
	void exportLoadsBack();
	void copySelectionAndColumns();
	void menuFollowsTheWorkFlow();
	void pathEnterLoads();
	void previewFollowsTheTable();
	void pushesTheTableClamped();

private:
	QString writeFile(const QString &name, const QString &text);
	int16_t pushed(int channel, int sample) const
	{
		return m_stream->lastPush.value(sample * m_stream->enabledChannels().size() + channel);
	}

	QTemporaryDir m_dir;
	FakeOutputStream *m_stream = nullptr;
	RawGeneratorView *m_view = nullptr;
};

void TST_RawGeneratorView::init()
{
	m_stream = new FakeOutputStream(2);
	m_view = new RawGeneratorView(m_stream);
}

void TST_RawGeneratorView::cleanup()
{
	delete m_view;
	m_view = nullptr;
	delete m_stream;
	m_stream = nullptr;
}

QString TST_RawGeneratorView::writeFile(const QString &name, const QString &text)
{
	const QString path = m_dir.filePath(name);
	QFile f(path);
	if(f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		f.write(text.toUtf8());
	}
	return path;
}

void TST_RawGeneratorView::parseHeaderAndIndexColumn()
{
	ParsedSamples p;
	QVERIFY(parseSamples("sample,voltage0,voltage1\n0,10,-20\n1,11.6,-21\n\n# note\n2,12,\n", p));
	QCOMPARE(p.header, QStringList({"voltage0", "voltage1"}));
	QCOMPARE(p.columns.size(), 2);
	QCOMPARE(p.rows(), 3);
	QCOMPARE(p.columns[0], QVector<double>({10, 12, 12}));	// 11.6 rounds to a code
	QCOMPARE(p.columns[1], QVector<double>({-20, -21, 0})); // an empty cell is 0
}

void TST_RawGeneratorView::parseSeparators()
{
	for(const QString &text :
	    {QStringLiteral("1;2\n3;4"), QStringLiteral("1\t2\n3\t4"), QStringLiteral("1 2\n3   4")}) {
		ParsedSamples p;
		QVERIFY2(parseSamples(text, p), qPrintable(text));
		QCOMPARE(p.columns, QVector<QVector<double>>({{1, 3}, {2, 4}}));
		QVERIFY(p.header.isEmpty());
	}
}

void TST_RawGeneratorView::parseRejectsText()
{
	ParsedSamples p;
	QString error;
	QVERIFY(!parseSamples("a,b\n1,2\n3,x\n", p, &error));
	QVERIFY2(error.contains("line 3"), qPrintable(error));
	QVERIFY(!parseSamples("\n\n", p, &error));
}

void TST_RawGeneratorView::tableEdits()
{
	SampleTableModel *m = m_view->samples();
	QCOMPARE(m->rowCount(), RawGeneratorView::DefaultRows);
	QCOMPARE(m->columnCount(), 2);
	QCOMPARE(m->headerData(1, Qt::Horizontal).toString(), QStringLiteral("voltage1"));
	QVERIFY(m->setData(m->index(3, 1), "123"));
	QCOMPARE(m->value(3, 1), 123.0);
	QVERIFY(!m->setData(m->index(3, 1), "abc")); // not a number: unchanged
	QCOMPARE(m->index(3, 1).data().toString(), QStringLiteral("123"));
}

void TST_RawGeneratorView::rowsFollowTheSpinbox()
{
	auto *rows = m_view->findChild<gui::MenuSpinbox *>("rawGeneratorRows");
	QVERIFY(rows);
	rows->setValue(16);
	QCOMPARE(m_view->samples()->rowCount(), 16);
	m_view->samples()->setRows(32);
	QCOMPARE(rows->value(), 32.0);
}

void TST_RawGeneratorView::loadMapsColumnsInOrder()
{
	QVERIFY(m_view->loadFile(writeFile("one.csv", "a\n5\n6\n7\n")));
	SampleTableModel *m = m_view->samples();
	QCOMPARE(m->rowCount(), 3);
	QCOMPARE(m->column(0), QVector<double>({5, 6, 7}));
	QCOMPARE(m->column(1), QVector<double>({0, 0, 0})); // no column for voltage1
	QCOMPARE(m_view->generation()->samples(), size_t(3));
	QVERIFY(!m_view->loadFile(m_dir.filePath("missing.csv")));
}

void TST_RawGeneratorView::loadIgnoresExtraColumns()
{
	QVERIFY(m_view->loadFile(writeFile("three.txt", "1 2 3\n4 5 6\n")));
	QCOMPARE(m_view->samples()->column(0), QVector<double>({1, 4}));
	QCOMPARE(m_view->samples()->column(1), QVector<double>({2, 5}));
}

void TST_RawGeneratorView::pasteGrowsTheTable()
{
	m_view->samples()->setRows(2);
	m_view->table()->setCurrentIndex(m_view->samples()->index(1, 1));
	QApplication::clipboard()->setText("7\t8\n9\t10\n");
	m_view->pasteClipboard();
	SampleTableModel *m = m_view->samples();
	QCOMPARE(m->rowCount(), 3);
	QCOMPARE(m->column(1), QVector<double>({0, 7, 9})); // the second pasted column is past the last channel
}

void TST_RawGeneratorView::exportLoadsBack()
{
	SampleTableModel *m = m_view->samples();
	m->setColumns({{1, 2, 3}, {-1, -2, -3}});
	const QString csv = m->toCsv();
	QVERIFY(csv.startsWith("sample,voltage0,voltage1\n0,1,-1\n"));
	ParsedSamples p;
	QVERIFY(parseSamples(csv, p));
	QCOMPARE(p.columns, QVector<QVector<double>>({{1, 2, 3}, {-1, -2, -3}}));
}

void TST_RawGeneratorView::previewFollowsTheTable()
{
	m_view->samples()->setColumns({{0, 100, 200, 300}, {0, -50, 0, 50}});
	m_view->apply();
	QCOMPARE(m_view->plot()->xAxis()->min(), 0.0);
	QCOMPARE(m_view->plot()->xAxis()->max(), 4.0);
	PlotChannel *ch0 = m_view->plot()->getChannels().at(0);
	QCOMPARE(int(ch0->curve()->dataSize()), 4);
	QCOMPARE(ch0->curve()->sample(2), QPointF(2, 200));
	QVERIFY(m_view->plot()->yAxis()->max() > 300);
}

void TST_RawGeneratorView::pushesTheTableClamped()
{
	m_view->samples()->setColumns({{1, 2, 40000, 4}, {-5, -6, -7, -40000}});
	m_view->apply();
	m_view->generation()->start();
	QTRY_VERIFY(m_stream->pushes >= 1);
	QCOMPARE(m_stream->enabledChannels(), QList<int>({0, 1}));
	QCOMPARE(pushed(0, 0), int16_t(1));
	QCOMPARE(pushed(0, 2), int16_t(32767)); // 16-bit signed channel
	QCOMPARE(pushed(1, 1), int16_t(-6));
	QCOMPARE(pushed(1, 3), int16_t(-32768));
	QCOMPARE(m_view->generation()->clippedSamples(), quint64(2));
	m_view->generation()->stop();
}

void TST_RawGeneratorView::copySelectionAndColumns()
{
	SampleTableModel *m = m_view->samples();
	m->setColumns({{1, 2, 3}, {-1, -2, -3}});
	QCOMPARE(m->toTsv(1, 2, 0, 1), QStringLiteral("2\t-2\n3\t-3\n"));

	// A cell range.
	QTableView *table = m_view->table();
	table->selectionModel()->select(QItemSelection(m->index(0, 0), m->index(1, 1)),
					QItemSelectionModel::ClearAndSelect);
	m_view->copySelection();
	QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("1\t-1\n2\t-2\n"));

	// A whole column, as a header click selects it; it pastes back.
	table->selectColumn(1);
	m_view->copySelection();
	QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("-1\n-2\n-3\n"));
	table->setCurrentIndex(m->index(0, 0));
	m_view->pasteClipboard();
	QCOMPARE(m->column(0), QVector<double>({-1, -2, -3}));
}

void TST_RawGeneratorView::menuFollowsTheWorkFlow()
{
	// Run in the top bar; the menu: Samples (path, Load, rows), then Output.
	auto *run = m_view->findChild<QPushButton *>("rawGeneratorRun");
	QVERIFY(run);
	QVERIFY(!m_view->findChild<QWidget *>("rawGeneratorSettingsMenu")->isAncestorOf(run));
	QWidget *samples = m_view->findChild<QWidget *>("rawGeneratorSamplesSection");
	QWidget *output = m_view->findChild<QWidget *>("rawGeneratorOutputSection");
	QVERIFY(samples && output);
	QLayout *lay = samples->parentWidget()->layout();
	QVERIFY(lay->indexOf(samples) < lay->indexOf(output));
	QVERIFY(samples->isAncestorOf(m_view->findChild<QPushButton *>("rawGeneratorLoad")));
	QVERIFY(samples->isAncestorOf(m_view->findChild<QWidget *>("rawGeneratorPath")));
	QVERIFY(samples->isAncestorOf(m_view->findChild<QWidget *>("rawGeneratorRows")));
	for(QPushButton *b : m_view->findChildren<QPushButton *>()) {
		QVERIFY2(!b->text().startsWith("Export"), qPrintable(b->text()));
	}
}

void TST_RawGeneratorView::pathEnterLoads()
{
	const QString path = writeFile("typed.csv", "9\n8\n");
	auto *edit = m_view->findChild<QWidget *>("rawGeneratorPath")->findChild<QLineEdit *>();
	QVERIFY(edit);
	edit->setText(path);
	Q_EMIT edit->returnPressed();
	QCOMPARE(m_view->samples()->column(0), QVector<double>({9, 8}));
}

QTEST_MAIN(TST_RawGeneratorView)
#include "tst_rawgeneratorview.moc"
