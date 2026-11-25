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

#include "dcdebuggertool.h"
#include "log/codegeneratorpanel.h"
#include "log/commandlogmodel.h"
#include "log/commandlogview.h"
#include "sidesplitter.h"

#include <QAbstractButton>
#include <QAction>
#include <QClipboard>
#include <QComboBox>
#include <QDir>
#include <QTabWidget>
#include <QFile>
#include <QGuiApplication>
#include <QFrame>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QSplitter>
#include <QTemporaryDir>
#include <QTreeView>
#include <QtTest>

#include <component/codeemitterregistry.h>
#include <dcdebugger/style_properties.h>
#include <core/codeemitter.h>
#include <iioutil/libiioemitter.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <pluginbase/preferences.h>
#include <style.h>
#include <tooltemplate.h>

#include <gui/docking/docksettings.h>
#ifdef USE_KDDOCKWIDGETS
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>
#endif

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

const QString C = QString::fromLatin1(codegen::C);
const QString PY = QString::fromLatin1(codegen::Python);
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

constexpr const char *DOCKING_PREF = "general_use_docking_if_available";

// The docking modes this build offers: classic always, KDDockWidgets when built with it.
void dockingModes()
{
	QTest::addColumn<bool>("docking");
	QTest::newRow("classic") << false;
#ifdef USE_KDDOCKWIDGETS
	QTest::newRow("kddockwidgets") << true;
#endif
}

QString dockTitle(QWidget *dock)
{
#ifdef USE_KDDOCKWIDGETS
	if(auto *kd = dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(dock)) {
		return kd->asDockWidgetController()->title().trimmed();
	}
#endif
	return dock->windowTitle();
}

// The Scopy theme, as the app loads it; false when the build has no style folder.
bool initScopyStyle()
{
	if(!QFile::exists(QStringLiteral("style/json/Scopy.json"))) {
		return false;
	}
	return Style::GetInstance()->init(QStringLiteral("Scopy"), 1.0);
}

bool isPlainWidget(const QWidget *w) { return w->metaObject() == &QWidget::staticMetaObject; }

// A plain QWidget under `root` is painted by any ".QWidget" background an ancestor cascades,
// unless it is part of a scroll area (the table, the preview: their own, darker colour) or is or
// sits in a ToolTemplate that resets ".QWidget" to transparent.
QStringList exposedPlainWidgets(QWidget *root)
{
	QStringList out;
	for(QWidget *w : root->findChildren<QWidget *>()) {
		if(!isPlainWidget(w)) {
			continue;
		}
		bool covered = false;
		for(QWidget *p = w; p && p != root->parentWidget(); p = p->parentWidget()) {
			if(qobject_cast<QAbstractScrollArea *>(p->parentWidget()) ||
			   (dynamic_cast<ToolTemplate *>(p) &&
			    p->styleSheet().contains(QStringLiteral("transparent")))) {
				covered = true;
				break;
			}
		}
		if(!covered) {
			out << QStringLiteral("%1 (parent %2)")
					.arg(w->objectName(), w->parentWidget()->metaObject()->className());
		}
	}
	return out;
}

// The colour the top level shows at the first point of `w` (clear of its rounded border) not
// covered by a child.
QColor panelColourAt(QWidget *top, QImage &shot, QWidget *w)
{
	for(int y = w->height() / 2; y < w->height() - 3; ++y) {
		for(int x = 3; x < w->width() - 3; ++x) {
			if(!w->childAt(x, y)) {
				return shot.pixelColor(w->mapTo(top, QPoint(x, y)) * shot.devicePixelRatio());
			}
		}
	}
	return {};
}

} // namespace

class TST_CodeGenerator : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void cleanup();
	void languagesFollowEntries();
	void emptyEntriesNothingToGenerate();
	void previewFollowsOptions();
	void copyPutsScriptOnClipboard();
	void saveWritesScript();

	void generateButtonTogglesPanel();
	void generateButtonInLogTopBar();
	void docks_data();
	void docks();
	void closingGeneratorUnchecksToggle_data();
	void closingGeneratorUnchecksToggle();
	void rowsAreFrames_data();
	void rowsAreFrames();
	void panelBackgrounds_data();
	void panelBackgrounds();
	void logTableGrid();
	void contextActionOpensPanel();
	void comboFollowsSelection();
	void callRowSelectsThatCall();
	void followsLogWithoutSelection();
};

void TST_CodeGenerator::initTestCase()
{
	CommandLogView::initPreferences();
	initDockWidgets(); // as main() does; a no-op without KDDockWidgets
	// Style::init finds the style in "./style" of the build folder.
	QDir::setCurrent(QStringLiteral(DCDEBUGGER_TEST_BUILD_DIR));
}

void TST_CodeGenerator::cleanup()
{
	Preferences::set(DOCKING_PREF, false);
	// The kept layout is process-wide: no test case starts from another's arrangement.
	layoutstate::setValues({});
}

void TST_CodeGenerator::languagesFollowEntries()
{
	auto reg = makeRegistry();
	CodeGeneratorPanel panel;
	panel.setRegistry(reg.get());
	panel.setEntries(threeReads());
	QCOMPARE(panel.languages(), (QStringList{C, PY}));
	QCOMPARE(panel.languageCombo()->combo()->count(), 2);
	QCOMPARE(panel.languageCombo()->combo()->itemText(0), QStringLiteral("C"));
	QCOMPARE(panel.languageCombo()->combo()->itemText(1), QStringLiteral("Python"));
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
	QCOMPARE(panel.languages(), (QStringList{C, PY}));
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

	panel.setLanguage(C);
	QVERIFY(panel.preview()->toPlainText().contains(QStringLiteral("int main(")));
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

void TST_CodeGenerator::generateButtonInLogTopBar()
{
	// In the log's top bar, right of Export; the log is the tool's Trace tab.
	DCDebuggerTool tool(nullptr);
	CommandLogView *view = tool.commandLog();
	ToolTemplate *logTool = nullptr;
	for(QWidget *w : view->logPane()->findChildren<QWidget *>(Qt::FindDirectChildrenOnly)) {
		logTool = logTool ? logTool : dynamic_cast<ToolTemplate *>(w);
	}
	QVERIFY(logTool);
	QVERIFY(logTool->topContainer()->isAncestorOf(view->generateButton()));
	QVERIFY(logTool->topContainer()->isAncestorOf(view->pauseButton()));
	QCOMPARE(view->generateButton()->parentWidget(), view->pauseButton()->parentWidget());
	QCOMPARE(view->generateButton()->text(), QStringLiteral("Generate code..."));
	QCOMPARE(tool.tabs()->widget(DCDebuggerTool::TraceTab), static_cast<QWidget *>(view));
}

void TST_CodeGenerator::docks_data() { dockingModes(); }

void TST_CodeGenerator::docks()
{
	QFETCH(bool, docking);
	Preferences::set(DOCKING_PREF, docking);
	CommandLogView view;
	view.resize(1400, 500);
	view.show();
	QVERIFY(QTest::qWaitForWindowExposed(&view));
	QCOMPARE(view.usesDocking(), docking);
	QVERIFY(view.dockArea()->isAncestorOf(view.logDock()));
	QVERIFY(view.logDock()->isAncestorOf(view.logPane()));
	QVERIFY(view.codeGeneratorDock()->isAncestorOf(view.codeGenerator()));
	QCOMPARE(dockTitle(view.logDock()), QStringLiteral("Command log"));
	QCOMPARE(dockTitle(view.codeGeneratorDock()), QStringLiteral("Generate code"));
	// The in-panel title only without a dock title bar.
	QCOMPARE(view.codeGenerator()->titleLabel()->isHidden(), docking);
	if(docking) {
		QVERIFY(!view.splitter());
#ifdef USE_KDDOCKWIDGETS
		// Their own area: the docks only dock with each other, not with the main window's.
		auto *area = dynamic_cast<KDDockWidgets::QtWidgets::MainWindow *>(view.dockArea());
		QVERIFY(area);
		for(QWidget *d : {view.logDock(), view.codeGeneratorDock()}) {
			auto *kd = dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(d);
			QVERIFY(kd);
			QCOMPARE(kd->asDockWidgetController()->affinities(), QStringList{area->uniqueName()});
		}
#endif
	} else {
		// A draggable splitter between the two wrappers.
		QVERIFY(view.splitter());
		QCOMPARE(view.splitter()->count(), 2);
		QCOMPARE(view.splitter()->widget(0), view.logDock());
		QCOMPARE(view.splitter()->widget(1), view.codeGeneratorDock());
		QVERIFY(view.splitter()->childrenCollapsible());
	}

	// The toggle shows and hides the generator dock.
	QVERIFY(!view.isCodeGeneratorShown());
	QVERIFY(!view.codeGenerator()->isVisible());
	view.generateButton()->click();
	QVERIFY(view.isCodeGeneratorShown());
	QTRY_VERIFY(view.codeGenerator()->isVisible());
	QVERIFY(view.dockArea()->isAncestorOf(view.codeGeneratorDock())); // in the same area
	QVERIFY(view.logPane()->isVisible());
	QVERIFY(view.codeGenerator()->width() > 0);
	view.generateButton()->click();
	QVERIFY(!view.isCodeGeneratorShown());
	QTRY_VERIFY(!view.codeGenerator()->isVisible());
	QVERIFY(view.logPane()->isVisible());
	view.generateAction()->trigger();
	QVERIFY(view.generateButton()->isChecked());
	QTRY_VERIFY(view.codeGenerator()->isVisible());
}

void TST_CodeGenerator::closingGeneratorUnchecksToggle_data() { dockingModes(); }

void TST_CodeGenerator::closingGeneratorUnchecksToggle()
{
	QFETCH(bool, docking);
	Preferences::set(DOCKING_PREF, docking);
	CommandLogView view;
	view.resize(1400, 500);
	view.show();
	QVERIFY(QTest::qWaitForWindowExposed(&view));
	view.showCodeGenerator(true);
	QVERIFY(view.generateButton()->isChecked());
	if(docking) {
#ifdef USE_KDDOCKWIDGETS
		// Closed by the layout, not through the toggle.
		dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(view.codeGeneratorDock())->close();
#endif
	} else {
		// Dragged fully closed.
		QSplitter *split = view.splitter();
		split->setSizes({split->width(), 0});
		Q_EMIT split->splitterMoved(split->width(), 1);
	}
	QVERIFY(!view.generateButton()->isChecked());
	QVERIFY(!view.isCodeGeneratorShown());
	// The toggle brings it back.
	view.generateButton()->click();
	QVERIFY(view.isCodeGeneratorShown());
	QTRY_VERIFY(view.codeGenerator()->isVisible());
	QTRY_VERIFY(view.codeGenerator()->width() > 0);
}

void TST_CodeGenerator::rowsAreFrames_data() { dockingModes(); }

void TST_CodeGenerator::rowsAreFrames()
{
	// The rows are QFrames (".QWidget" does not match them) without a box of their own.
	QFETCH(bool, docking);
	Preferences::set(DOCKING_PREF, docking);
	CommandLogView view;
	const QList<QFrame *> rows = {
		view.switchesRow(), view.findChild<QFrame *>(QStringLiteral("commandLogFilters")),
		view.codeGenerator()->findChild<QFrame *>(QStringLiteral("codeGeneratorOptions")),
		view.codeGenerator()->findChild<QFrame *>(QStringLiteral("codeGeneratorButtons"))};
	for(QFrame *row : rows) {
		QVERIFY(row);
		QVERIFY(!isPlainWidget(row));
		QVERIFY(!row->property(style::properties::widget::basicComponent).toBool()); // no box per row
		QVERIFY(row->styleSheet().isEmpty());
	}
	// The panels paint their style sheet background.
	for(QFrame *panel : {view.logPane(), static_cast<QFrame *>(view.codeGenerator())}) {
		QVERIFY(panel->testAttribute(Qt::WA_StyledBackground));
	}
	// No plain container is left for a cascaded ".QWidget" background to paint.
	QVERIFY2(exposedPlainWidgets(view.logPane()).isEmpty(),
		 qPrintable(exposedPlainWidgets(view.logPane()).join(", ")));
	QVERIFY2(exposedPlainWidgets(view.codeGenerator()).isEmpty(),
		 qPrintable(exposedPlainWidgets(view.codeGenerator()).join(", ")));
}

void TST_CodeGenerator::panelBackgrounds_data() { dockingModes(); }

void TST_CodeGenerator::panelBackgrounds()
{
	// In the tool (whose ToolTemplate cascades ".QWidget { background_subtle }"): every row
	// shows the panel colour.
	QFETCH(bool, docking);
	if(!initScopyStyle()) {
		QSKIP("No style folder in the build");
	}
	Preferences::set(DOCKING_PREF, docking);
	DCDebuggerTool tool(nullptr);
	tool.resize(1600, 1000);
	tool.show();
	QVERIFY(QTest::qWaitForWindowExposed(&tool));
	tool.showTab(DCDebuggerTool::TraceTab);
	CommandLogView *view = tool.commandLog();
	view->showCodeGenerator(true);
	QTest::qWait(100);
	QImage shot = tool.grab().toImage();
	const QColor panel = Style::getColor(json::theme::background_primary);
	QVERIFY(panel != Style::getColor(json::theme::background_subtle));
	const QList<QWidget *> rows = {
		view->logPane(),
		view->switchesRow(),
		view->findChild<QFrame *>(QStringLiteral("commandLogFilters")),
		view->codeGenerator(),
		view->codeGenerator()->findChild<QFrame *>(QStringLiteral("codeGeneratorOptions")),
		view->codeGenerator()->findChild<QFrame *>(QStringLiteral("codeGeneratorButtons"))};
	for(QWidget *row : rows) {
		QVERIFY(row && row->isVisible());
		QCOMPARE(panelColourAt(&tool, shot, row).name(), panel.name());
	}
}

void TST_CodeGenerator::logTableGrid()
{
	// The table: the pane colour, grid lines between the cells and the header sections.
	if(!initScopyStyle()) {
		QSKIP("No style folder in the build");
	}
	const QString qss = qApp->styleSheet();
	const QString sel = QStringLiteral("QTreeView[%1=true]").arg(style::properties::dcdebugger::logTable);
	auto rule = [&](const QString &selector) {
		const int at = qss.indexOf(selector + QStringLiteral(" {"));
		return at < 0 ? QString() : qss.mid(at, qss.indexOf(QLatin1Char('}'), at) - at);
	};
	const QString grid = Style::getAttribute(json::theme::interactive_subtle_disabled);
	QVERIFY2(rule(sel).contains(QStringLiteral("background-color: ") +
				    Style::getAttribute(json::theme::background_primary)),
		 qPrintable(rule(sel)));
	const QString item = rule(sel + QStringLiteral("::item"));
	QVERIFY2(item.contains(QStringLiteral("border-right: 1px solid ") + grid), qPrintable(item));
	QVERIFY2(item.contains(QStringLiteral("border-bottom: 1px solid ") + grid), qPrintable(item));
	const QString section = rule(sel + QStringLiteral(" QHeaderView::section"));
	QVERIFY2(section.contains(QStringLiteral("border-right: 1px solid ") + grid), qPrintable(section));
	// Selection and expand arrows kept.
	QVERIFY(!rule(sel + QStringLiteral("::item:selected")).isEmpty());
	QVERIFY(qss.contains(sel + QStringLiteral("::branch:closed:has-children:has-siblings")));
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
	QCOMPARE(panel->languages(), (QStringList{C, PY}));
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
