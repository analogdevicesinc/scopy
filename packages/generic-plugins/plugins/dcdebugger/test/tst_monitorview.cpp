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
#include "monitor/monitorview.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtTest>

#include <component/channel.h>
#include <component/device.h>
#include <menucollapsesection.h>
#include <menucontrolbutton.h>
#include <menusectionwidget.h>
#include <plotchannel.h>
#include <plotwidget.h>
#include <toolbuttons.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// Runtime plots of the Polling monitor: one plot at start, add / remove from the Settings menu's
// Plots section (plot combo, Add plot, Remove plot), the per-channel plot combos follow.
class TST_MonitorView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void settingsButtonNamedSettings();
	void settingsMenuHasPollingAndPlots();
	void startsWithOnePlot();
	void addPlotUpdatesCombos();
	void addSelectsNewPlot();
	void removeSelectedPlot();
	void removeKeepsValidSelection();
	void programmaticChangesSyncPlotsSection();
	void removePlotMovesCurvesToFirst();
	void removeFirstPlotKeepsTheOthers();
	void lastPlotCannotBeRemoved();
	void maxPlots();
	void assignmentSurvivesRemovingALaterPlot();
	void plotVisibleFromStart();

private:
	QComboBox *combo(const QString &key) const;
	QPushButton *button(const QString &name) const;
	QComboBox *plotSelect() const;
	QWidget *settingsMenu() const;
	QStringList comboItems(const QString &key) const;
	int curves(int plot) const;

	Device *m_dev = nullptr;
	MonitorView *m_view = nullptr;
};

namespace {
const QString TEMP0 = QStringLiteral("adc/temp0");
const QString TEMP1 = QStringLiteral("adc/temp1");
const QString VOLT0 = QStringLiteral("adc/voltage0");
} // namespace

void TST_MonitorView::init()
{
	m_dev = new Device();
	m_dev->setName("adc");
	for(const char *id : {"temp0", "temp1", "voltage0"}) {
		auto *ch = new Channel(m_dev);
		ch->setId(id);
		makeAttribute(ch, "input", "1");
	}
	m_view = new MonitorView(m_dev);
}

void TST_MonitorView::cleanup()
{
	delete m_view;
	m_view = nullptr;
	delete m_dev;
	m_dev = nullptr;
}

QComboBox *TST_MonitorView::combo(const QString &key) const
{
	return m_view->findChild<QComboBox *>("monitorPlotCombo:" + key);
}

QPushButton *TST_MonitorView::button(const QString &name) const { return m_view->findChild<QPushButton *>(name); }

QComboBox *TST_MonitorView::plotSelect() const { return m_view->findChild<QComboBox *>("monitorPlotSelect"); }

QWidget *TST_MonitorView::settingsMenu() const { return m_view->findChild<QWidget *>("monitorSettingsMenu"); }

QStringList TST_MonitorView::comboItems(const QString &key) const
{
	QStringList out;
	QComboBox *c = combo(key);
	for(int i = 0; c && i < c->count(); ++i) {
		out.append(c->itemText(i));
	}
	return out;
}

int TST_MonitorView::curves(int plot) const
{
	PlotWidget *p = m_view->plot(plot);
	return p ? int(p->getChannels().size()) : -1;
}

void TST_MonitorView::settingsButtonNamedSettings()
{
	auto *btn = m_view->findChild<MenuControlButton *>("monitorSettingsButton");
	QVERIFY(btn);
	QStringList texts;
	for(QLabel *l : btn->findChildren<QLabel *>()) {
		texts.append(l->text());
	}
	QVERIFY(texts.contains(QStringLiteral("Settings")));
	QVERIFY(!texts.contains(QStringLiteral("Polling")));
}

void TST_MonitorView::settingsMenuHasPollingAndPlots()
{
	QWidget *menu = settingsMenu();
	QVERIFY(menu);
	// No gear and no plot-count bar: the Plots section is always there.
	QVERIFY(menu->findChildren<GearBtn *>().isEmpty());
	QVERIFY(!m_view->findChild<QObject *>("monitorPlotSettings"));

	auto *polling = menu->findChild<MenuSectionCollapseWidget *>("monitorPollingSection");
	auto *plots = menu->findChild<MenuSectionCollapseWidget *>("monitorPlotsSection");
	QVERIFY(polling);
	QVERIFY(plots);
	QCOMPARE(polling->collapseSection()->title(), QStringLiteral("Polling"));
	QCOMPARE(plots->collapseSection()->title(), QStringLiteral("Plots"));
	QVERIFY(!plots->isHidden());

	// Plots section, top to bottom: plot combo, Add plot, Remove plot.
	QVBoxLayout *lay = plots->contentLayout();
	auto *comboMenu = plots->findChild<QWidget *>("monitorPlotSelectMenu");
	QVERIFY(comboMenu);
	QVERIFY(plotSelect());
	QVERIFY(comboMenu->isAncestorOf(plotSelect()));
	const int comboAt = lay->indexOf(comboMenu);
	const int addAt = lay->indexOf(button("monitorAddPlot"));
	const int removeAt = lay->indexOf(button("monitorRemovePlot"));
	QVERIFY(comboAt >= 0);
	QCOMPARE(addAt, comboAt + 1);
	QCOMPARE(removeAt, addAt + 1);
	QCOMPARE(button("monitorAddPlot")->text(), QStringLiteral("Add plot"));
	QCOMPARE(button("monitorRemovePlot")->text(), QStringLiteral("Remove plot"));
	// The old per-plot rows are gone.
	QVERIFY(!m_view->findChild<QPushButton *>("monitorRemovePlot1"));
}

void TST_MonitorView::startsWithOnePlot()
{
	QCOMPARE(m_view->plotCount(), 1);
	QVERIFY(m_view->plot(0));
	QVERIFY(!m_view->plot(1));
	for(const QString &key : {TEMP0, TEMP1, VOLT0}) {
		QVERIFY(combo(key));
		QCOMPARE(comboItems(key), QStringList({"Plot 1"}));
		QCOMPARE(combo(key)->currentIndex(), 0);
		QCOMPARE(m_view->plotIndexOf(key), 0); // auto-assignment stays on the existing plot
	}
	QCOMPARE(m_view->plotIndexOf("nope"), -1);
	QCOMPARE(curves(0), 3);
	QCOMPARE(plotSelect()->count(), 1);
	QCOMPARE(plotSelect()->itemText(0), QStringLiteral("Plot 1"));
	QCOMPARE(plotSelect()->currentIndex(), 0);
	QVERIFY(button("monitorAddPlot")->isEnabled());
	QVERIFY(button("monitorRemovePlot")->isHidden());
}

void TST_MonitorView::addPlotUpdatesCombos()
{
	QSignalSpy changed(m_view, &MonitorView::plotsChanged);
	QCOMPARE(m_view->addPlot(), 1);
	QCOMPARE(m_view->plotCount(), 2);
	QCOMPARE(changed.count(), 1);
	QCOMPARE(changed.last().at(0).toInt(), 2);
	for(const QString &key : {TEMP0, TEMP1, VOLT0}) {
		QCOMPARE(comboItems(key), QStringList({"Plot 1", "Plot 2"}));
		QCOMPARE(combo(key)->currentIndex(), 0); // selection kept
	}
	QCOMPARE(curves(0), 3);
	QCOMPARE(curves(1), 0);

	// The Plots section's button does the same.
	button("monitorAddPlot")->click();
	QCOMPARE(m_view->plotCount(), 3);
	QCOMPARE(comboItems(TEMP0).size(), 3);
	QCOMPARE(plotSelect()->count(), 3);
	QVERIFY(!button("monitorRemovePlot")->isHidden());
}

void TST_MonitorView::addSelectsNewPlot()
{
	button("monitorAddPlot")->click();
	QCOMPARE(m_view->plotCount(), 2);
	QCOMPARE(plotSelect()->count(), 2);
	QCOMPARE(plotSelect()->itemText(1), QStringLiteral("Plot 2"));
	QCOMPARE(plotSelect()->currentIndex(), 1);
	QVERIFY(!button("monitorRemovePlot")->isHidden());
	// The channels' combos keep their selection.
	QCOMPARE(combo(TEMP0)->currentIndex(), 0);
}

void TST_MonitorView::removeSelectedPlot()
{
	button("monitorAddPlot")->click();
	combo(TEMP1)->setCurrentIndex(1);
	QCOMPARE(curves(1), 1);

	QSignalSpy changed(m_view, &MonitorView::plotsChanged);
	plotSelect()->setCurrentIndex(1); // Plot 2
	button("monitorRemovePlot")->click();
	QCOMPARE(changed.count(), 1);
	QCOMPARE(m_view->plotCount(), 1);
	QCOMPARE(plotSelect()->count(), 1);
	QCOMPARE(plotSelect()->currentIndex(), 0);
	QVERIFY(button("monitorRemovePlot")->isHidden());
	QCOMPARE(curves(0), 3);
	for(const QString &key : {TEMP0, TEMP1, VOLT0}) {
		QCOMPARE(m_view->plotIndexOf(key), 0);
		QCOMPARE(comboItems(key), QStringList({"Plot 1"}));
	}
}

void TST_MonitorView::removeKeepsValidSelection()
{
	m_view->addPlot();
	m_view->addPlot();
	m_view->addPlot(); // 4 plots, Plot 4 selected
	QCOMPARE(plotSelect()->currentIndex(), 3);

	// Removing a middle plot selects the one now in its place.
	plotSelect()->setCurrentIndex(1);
	button("monitorRemovePlot")->click();
	QCOMPARE(m_view->plotCount(), 3);
	QCOMPARE(plotSelect()->count(), 3);
	QCOMPARE(plotSelect()->currentIndex(), 1);

	// Removing the last plot selects the previous one.
	plotSelect()->setCurrentIndex(2);
	button("monitorRemovePlot")->click();
	QCOMPARE(m_view->plotCount(), 2);
	QCOMPARE(plotSelect()->currentIndex(), 1);

	// Removing the first plot keeps a valid selection too.
	plotSelect()->setCurrentIndex(0);
	button("monitorRemovePlot")->click();
	QCOMPARE(m_view->plotCount(), 1);
	QCOMPARE(plotSelect()->count(), 1);
	QCOMPARE(plotSelect()->currentIndex(), 0);
	QVERIFY(button("monitorRemovePlot")->isHidden());
	QCOMPARE(curves(0), 3);
}

void TST_MonitorView::programmaticChangesSyncPlotsSection()
{
	plotSelect()->setCurrentIndex(0);
	m_view->addPlot();
	m_view->addPlot();
	plotSelect()->setCurrentIndex(0);
	// Removing another plot than the selected one keeps the selection on the same plot.
	QVERIFY(m_view->removePlot(2));
	QCOMPARE(plotSelect()->count(), 2);
	QCOMPARE(plotSelect()->currentIndex(), 0);
	QVERIFY(!button("monitorRemovePlot")->isHidden());
	QVERIFY(m_view->removePlot(1));
	QCOMPARE(plotSelect()->count(), 1);
	QVERIFY(button("monitorRemovePlot")->isHidden());
}

void TST_MonitorView::removePlotMovesCurvesToFirst()
{
	m_view->addPlot();
	combo(TEMP1)->setCurrentIndex(1);
	combo(VOLT0)->setCurrentIndex(1);
	QCOMPARE(m_view->plotIndexOf(TEMP1), 1);
	QCOMPARE(curves(0), 1);
	QCOMPARE(curves(1), 2);

	plotSelect()->setCurrentIndex(1);
	button("monitorRemovePlot")->click();
	QCOMPARE(m_view->plotCount(), 1);
	QCOMPARE(curves(0), 3);
	for(const QString &key : {TEMP0, TEMP1, VOLT0}) {
		QCOMPARE(m_view->plotIndexOf(key), 0);
		QCOMPARE(comboItems(key), QStringList({"Plot 1"}));
		QCOMPARE(combo(key)->currentIndex(), 0);
	}
	// The moved curves draw on the remaining plot's axes.
	for(PlotChannel *ch : m_view->plot(0)->getChannels()) {
		QCOMPARE(ch->yAxis(), m_view->plot(0)->yAxis());
	}
}

void TST_MonitorView::removeFirstPlotKeepsTheOthers()
{
	m_view->addPlot();
	m_view->addPlot();
	combo(TEMP1)->setCurrentIndex(2);
	QVERIFY(m_view->removePlot(0));
	QCOMPARE(m_view->plotCount(), 2);
	// Plot 1's channels went to the new first plot (old Plot 2); old Plot 3 is now Plot 2.
	QCOMPARE(m_view->plotIndexOf(TEMP0), 0);
	QCOMPARE(m_view->plotIndexOf(VOLT0), 0);
	QCOMPARE(m_view->plotIndexOf(TEMP1), 1);
	QCOMPARE(combo(TEMP1)->currentText(), QStringLiteral("Plot 2"));
	QCOMPARE(curves(0), 2);
	QCOMPARE(curves(1), 1);
}

void TST_MonitorView::lastPlotCannotBeRemoved()
{
	QSignalSpy changed(m_view, &MonitorView::plotsChanged);
	QVERIFY(!m_view->removePlot(0));
	QVERIFY(!m_view->removePlot(1));
	QVERIFY(!m_view->removePlot(-1));
	QVERIFY(button("monitorRemovePlot")->isHidden()); // nothing to remove
	QCOMPARE(m_view->plotCount(), 1);
	QCOMPARE(changed.count(), 0);

	m_view->addPlot();
	QVERIFY(!button("monitorRemovePlot")->isHidden());
	QVERIFY(m_view->removePlot(1));
	QVERIFY(button("monitorRemovePlot")->isHidden());
	QVERIFY(!m_view->removePlot(0));
	QCOMPARE(m_view->plotCount(), 1);
	QCOMPARE(curves(0), 3);
}

void TST_MonitorView::maxPlots()
{
	for(int p = 1; p < MonitorView::MaxPlots; ++p) {
		QCOMPARE(m_view->addPlot(), p);
	}
	QCOMPARE(m_view->plotCount(), MonitorView::MaxPlots);
	QVERIFY(!button("monitorAddPlot")->isEnabled());
	QCOMPARE(plotSelect()->count(), MonitorView::MaxPlots);
	QCOMPARE(plotSelect()->currentIndex(), MonitorView::MaxPlots - 1);
	button("monitorAddPlot")->click(); // disabled: no effect
	QCOMPARE(m_view->addPlot(), -1);
	QCOMPARE(m_view->plotCount(), MonitorView::MaxPlots);
	QCOMPARE(comboItems(TEMP0).size(), MonitorView::MaxPlots);
	QCOMPARE(comboItems(TEMP0).last(), QStringLiteral("Plot %1").arg(MonitorView::MaxPlots));

	QVERIFY(m_view->removePlot(MonitorView::MaxPlots - 1));
	QVERIFY(button("monitorAddPlot")->isEnabled());
	QCOMPARE(comboItems(TEMP0).size(), MonitorView::MaxPlots - 1);
}

void TST_MonitorView::assignmentSurvivesRemovingALaterPlot()
{
	m_view->addPlot();
	m_view->addPlot();
	combo(TEMP1)->setCurrentIndex(1); // Plot 2
	combo(VOLT0)->setCurrentIndex(2); // Plot 3
	QCOMPARE(curves(1), 1);
	QCOMPARE(curves(2), 1);

	QVERIFY(m_view->removePlot(2));
	QCOMPARE(m_view->plotIndexOf(TEMP1), 1);
	QCOMPARE(combo(TEMP1)->currentIndex(), 1);
	QCOMPARE(comboItems(TEMP1), QStringList({"Plot 1", "Plot 2"}));
	QCOMPARE(m_view->plotIndexOf(VOLT0), 0); // its plot is gone
	QCOMPARE(combo(VOLT0)->currentIndex(), 0);
	QCOMPARE(curves(0), 2);
	QCOMPARE(curves(1), 1);
}

void TST_MonitorView::plotVisibleFromStart()
{
	m_view->resize(1200, 800);
	m_view->show();
	QVERIFY(QTest::qWaitForWindowExposed(m_view));
	QTableWidget *table = nullptr;
	for(QTableWidget *t : m_view->findChildren<QTableWidget *>()) {
		if(t->columnCount() == 5) {
			table = t;
		}
	}
	QVERIFY(table);
	PlotWidget *plot = m_view->plot(0);
	// Visible before any channel is enabled or polling started.
	QTRY_VERIFY(plot->isVisible() && plot->height() > 0);
	QVERIFY(plot->height() > table->height());

	// Runtime plots show up too.
	m_view->addPlot();
	QTRY_VERIFY(m_view->plot(1)->isVisible() && m_view->plot(1)->height() > 0);
}

QTEST_MAIN(TST_MonitorView)
#include "tst_monitorview.moc"
