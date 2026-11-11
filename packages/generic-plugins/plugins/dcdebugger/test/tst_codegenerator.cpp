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

#include "log/codegeneratorpanel.h"
#include "log/commandlogmodel.h"
#include "log/commandlogview.h"

#include <QAbstractButton>
#include <QAction>
#include <QClipboard>
#include <QComboBox>
#include <QFile>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTreeView>
#include <QtTest>

#include <component/codeemitterregistry.h>
#include <core/codeemitter.h>
#include <iioutil/libiioemitter.h>
#include <menucombo.h>
#include <menuonoffswitch.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

const QString C = QString::fromLatin1(codegen::C);
const QString PY = QString::fromLatin1(codegen::Python);
const QString CS = QString::fromLatin1(codegen::CSharp);
constexpr qint64 T0 = 1'767'225'600'000'000'000LL;

// A library with a C emitter only.
class OtherEmitter : public CodeEmitter
{
public:
	QString library() const override { return QStringLiteral("other"); }
	QString language() const override { return QString::fromLatin1(codegen::C); }
	QStringList headers(EmitContext &) const override { return {}; }
	QString prologue(EmitContext &) const override { return QStringLiteral("other_init();"); }
	QString emitCall(const CallRecord &call, EmitContext &) const override { return call.function + "();"; }
	QString epilogue(EmitContext &) const override { return {}; }
};

CallRecord gainRead(const QString &value)
{
	CallRecord r;
	r.library = QStringLiteral("libiio");
	r.function = QStringLiteral("iio_attr_read_raw");
	r.handle = 0xa000;
	r.args = {CallArg::string("attr", QStringLiteral("hardwaregain")), CallArg::outLen("len", 4096)};
	r.rc = int(value.size()) + 1;
	r.out = value;
	r.outKind = CallArg::String;
	r.outSize = value.size();
	return r;
}

CommandLogEntry entry(const QList<CallRecord> &calls, int n)
{
	CommandLogEntry e;
	e.origin = QStringLiteral("ctx/ad9361-phy/voltage0/hardwaregain");
	e.uri = QStringLiteral("ip:a");
	e.backendId = QStringLiteral("iio");
	e.commandClass = QStringLiteral("scopy::iio::AttrReadCommand");
	e.startNs = T0 + n * 1'000'000LL;
	e.calls = calls;
	e.originScope = {{originscope::Device, "ad9361-phy"},
			 {originscope::Channel, "voltage0"},
			 {originscope::Output, false},
			 {originscope::Attribute, "hardwaregain"},
			 {originscope::AttributeOf, "channel"}};
	return e;
}

CommandLogEntry otherEntry(int n)
{
	CommandLogEntry e;
	e.origin = QStringLiteral("dongle/i2c0");
	e.uri = QStringLiteral("ip:a");
	e.startNs = T0 + n * 1'000'000LL;
	CallRecord r;
	r.library = QStringLiteral("other");
	r.function = QStringLiteral("other_i2c_write");
	e.calls = {r};
	return e;
}

// Three reads of the same attribute: a loop of 3 with loops on.
QList<CommandLogEntry> threeReads()
{
	return {entry({gainRead("1")}, 0), entry({gainRead("1")}, 1), entry({gainRead("1")}, 2)};
}

std::unique_ptr<component::CodeEmitterRegistry> makeRegistry()
{
	auto r = std::make_unique<component::CodeEmitterRegistry>();
	for(const auto &e : iio::LibiioEmitter::create(iio::LibiioVersion::V1)) {
		r->add(e);
	}
	r->add(std::make_shared<OtherEmitter>());
	return r;
}

void select(QTreeView *tree, const QModelIndex &index, bool add = false)
{
	tree->selectionModel()->select(index,
				       (add ? QItemSelectionModel::Select : QItemSelectionModel::ClearAndSelect) |
					       QItemSelectionModel::Rows);
}

} // namespace

class TST_CodeGenerator : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void languagesFollowEntries();
	void emptyEntriesNothingToGenerate();
	void previewFollowsOptions();
	void copyPutsScriptOnClipboard();
	void saveWritesScript();

	void generateButtonTogglesPanel();
	void contextActionOpensPanel();
	void comboFollowsSelection();
	void callRowSelectsThatCall();
	void followsLogWithoutSelection();
};

void TST_CodeGenerator::languagesFollowEntries()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries(threeReads());
	QCOMPARE(panel.languages(), (QStringList{C, PY, CS}));
	QCOMPARE(panel.languageCombo()->combo()->itemText(2), QStringLiteral("C#"));
	panel.setLanguage(PY);
	QCOMPARE(panel.language(), PY);
	QVERIFY(panel.script().contains(QStringLiteral("import iio")));

	// libiio + other (C only): only C.
	panel.setEntries(threeReads() << otherEntry(5));
	QCOMPARE(panel.languages(), QStringList{C});
	QCOMPARE(panel.language(), C);
	QVERIFY(panel.script().contains(QStringLiteral("other_init();")));
	QVERIFY(panel.script().contains(QStringLiteral("iio_attr_read_raw(")));

	// Back to libiio only: the current language (C) stays.
	panel.setEntries(threeReads());
	QCOMPARE(panel.languages(), (QStringList{C, PY, CS}));
	QCOMPARE(panel.language(), C);
}

void TST_CodeGenerator::emptyEntriesNothingToGenerate()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries({});
	QVERIFY(panel.languages().isEmpty());
	QVERIFY(!panel.languageCombo()->combo()->isEnabled());
	QVERIFY(panel.script().isEmpty());
	QVERIFY(panel.preview()->toPlainText().isEmpty());
	QVERIFY(!panel.copyButton()->isEnabled());
	QVERIFY(!panel.saveButton()->isEnabled());
	QGuiApplication::clipboard()->setText(QStringLiteral("keep"));
	panel.copyToClipboard();
	QCOMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("keep"));
}

void TST_CodeGenerator::previewFollowsOptions()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries(threeReads());
	panel.setLanguage(PY);
	QVERIFY(panel.preview()->isReadOnly());
	QCOMPARE(panel.preview()->toPlainText(), panel.script());
	QVERIFY(panel.loopsSwitch()->onOffswitch()->isChecked());
	QVERIFY(panel.script().contains(QStringLiteral("for _ in range(3):")));

	QSignalSpy spy(&panel, &CodeGeneratorPanel::scriptChanged);
	panel.loopsSwitch()->onOffswitch()->setChecked(false);
	QCOMPARE(spy.count(), 1);
	QVERIFY(!panel.preview()->toPlainText().contains(QStringLiteral("for _ in range")));
	QCOMPARE(panel.preview()->toPlainText().count(QStringLiteral("val = attr0.value")), 3);
	QVERIFY(!panel.options().loops);

	panel.setLanguage(CS);
	QVERIFY(panel.preview()->toPlainText().contains(QStringLiteral("class Program")));
	QCOMPARE(panel.preview()->toPlainText(), panel.script());
}

void TST_CodeGenerator::copyPutsScriptOnClipboard()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries(threeReads());
	QVERIFY(panel.copyButton()->isEnabled());
	QGuiApplication::clipboard()->clear();
	panel.copyButton()->click();
	QCOMPARE(QGuiApplication::clipboard()->text(), panel.script());
	QVERIFY(QGuiApplication::clipboard()->text().contains(QStringLiteral("iio_create_context(NULL, \"ip:a\")")));
}

void TST_CodeGenerator::saveWritesScript()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries(threeReads());
	panel.setLanguage(PY);
	QTemporaryDir dir;
	const QString path = dir.filePath(QStringLiteral("script.py"));
	QVERIFY(panel.saveTo(path));
	QFile f(path);
	QVERIFY(f.open(QIODevice::ReadOnly));
	QCOMPARE(QString::fromUtf8(f.readAll()), panel.script());
	panel.setEntries({});
	QVERIFY(!panel.saveTo(dir.filePath(QStringLiteral("empty.py"))));
}

void TST_CodeGenerator::generateButtonTogglesPanel()
{
	CommandLogView view;
	QVERIFY(view.generateButton()->isEnabled());
	QVERIFY(view.generateButton()->isCheckable());
	QVERIFY(!view.isCodeGeneratorShown());
	view.generateButton()->click();
	QVERIFY(view.isCodeGeneratorShown());
	view.generateButton()->click();
	QVERIFY(!view.isCodeGeneratorShown());
	view.showCodeGenerator(true);
	QVERIFY(view.generateButton()->isChecked());
}

void TST_CodeGenerator::contextActionOpensPanel()
{
	CommandLogView view;
	QCOMPARE(view.generateAction()->text(), QStringLiteral("Generate code..."));
	view.generateAction()->trigger();
	QVERIFY(view.isCodeGeneratorShown());
	QVERIFY(view.generateButton()->isChecked());
}

void TST_CodeGenerator::comboFollowsSelection()
{
	auto reg = makeRegistry();
	CommandLogView view;
	view.codeGenerator()->setRegistry(reg.get());
	view.model()->setCollapseAfter(0);
	view.model()->append(entry({gainRead("1")}, 0));
	view.model()->append(otherEntry(1));
	view.model()->append(entry({gainRead("2")}, 2));
	view.showCodeGenerator(true);
	CodeGeneratorPanel *panel = view.codeGenerator();

	// Nothing selected: every visible row (both libraries) → C only.
	QCOMPARE(panel->entries().size(), 3);
	QCOMPARE(panel->languages(), QStringList{C});
	QVERIFY(panel->scopeLabel()->text().contains(QStringLiteral("Every visible row")));

	// A libiio row: every language.
	select(view.tree(), view.model()->index(0, 0));
	QCOMPARE(panel->entries().size(), 1);
	QCOMPARE(panel->languages(), (QStringList{C, PY, CS}));
	panel->setLanguage(PY);
	QVERIFY(panel->script().contains(QStringLiteral("val = attr0.value")));
	QVERIFY(!panel->script().contains(QStringLiteral("other")));

	// Add the other library's row: C only again.
	select(view.tree(), view.model()->index(1, 0), true);
	QCOMPARE(panel->entries().size(), 2);
	QCOMPARE(panel->languages(), QStringList{C});
	QVERIFY(panel->scopeLabel()->text().contains(QStringLiteral("2 selected row(s)")));

	// Cleared: back to every row.
	view.tree()->selectionModel()->clearSelection();
	QCOMPARE(panel->entries().size(), 3);
	QCOMPARE(view.generationEntries().size(), 3);
}

void TST_CodeGenerator::callRowSelectsThatCall()
{
	auto reg = makeRegistry();
	CommandLogView view;
	view.codeGenerator()->setRegistry(reg.get());
	CallRecord second = gainRead("2");
	second.function = QStringLiteral("iio_attr_read_raw");
	view.model()->append(entry({gainRead("1"), second, gainRead("3")}, 0));
	view.showCodeGenerator(true);
	const QModelIndex top = view.model()->index(0, 0);
	QCOMPARE(view.model()->rowCount(top), 3);
	view.tree()->expand(top);
	select(view.tree(), view.model()->index(2, 0, top));
	QList<CommandLogEntry> picked = view.generationEntries();
	QCOMPARE(picked.size(), 1);
	QCOMPARE(picked[0].calls.size(), 1);
	QCOMPARE(picked[0].calls[0].out.toString(), QStringLiteral("3"));
	// The command row as well: the whole command.
	select(view.tree(), top, true);
	picked = view.generationEntries();
	QCOMPARE(picked.size(), 1);
	QCOMPARE(picked[0].calls.size(), 3);
	QCOMPARE(view.codeGenerator()->entries().first().calls.size(), 3);
	// A call row under a collapsed command is not shown: not used.
	view.tree()->collapse(top);
	select(view.tree(), view.model()->index(0, 0, top));
	QCOMPARE(view.generationEntries().size(), 1); // nothing shown selected → every visible row
	QCOMPARE(view.generationEntries()[0].calls.size(), 3);
}

void TST_CodeGenerator::followsLogWithoutSelection()
{
	auto reg = makeRegistry();
	CommandLogView view;
	view.codeGenerator()->setRegistry(reg.get());
	view.model()->setCollapseAfter(0);
	view.model()->append(entry({gainRead("1")}, 0));
	// Hidden: not fed.
	QVERIFY(view.codeGenerator()->entries().isEmpty());
	view.showCodeGenerator(true);
	QCOMPARE(view.codeGenerator()->entries().size(), 1);
	view.model()->append(entry({gainRead("1")}, 1));
	QTRY_COMPARE(view.codeGenerator()->entries().size(), 2);
	QVERIFY(view.codeGenerator()->script().contains(QStringLiteral("2 times")));
	view.model()->clear();
	QTRY_VERIFY(view.codeGenerator()->entries().isEmpty());
	QVERIFY(view.codeGenerator()->script().isEmpty());
}

QTEST_MAIN(TST_CodeGenerator)
#include "tst_codegenerator.moc"
