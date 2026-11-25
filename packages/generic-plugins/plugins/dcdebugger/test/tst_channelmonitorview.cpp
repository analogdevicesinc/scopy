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
#include "monitor/channelmonitorview.h"

#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QtTest>

#include <component/channel.h>
#include <component/device.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// Table-only monitor: every input channel with the selected attribute is polled, raw values.
class TST_ChannelMonitorView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void onlyInputChannels();
	void defaultsToRaw();
	void attributeComboListsNames();
	void changingAttributeChangesRows();
	void pollsRawWithoutScale();
	void noDeviceColumn();
	void periodDefaultsToOneSecond();
	void contextHasNoChannels();

private:
	QTableWidget *table() const { return m_view->findChild<QTableWidget *>("channelMonitorTable"); }
	QComboBox *attrCombo() const { return m_view->findChild<QComboBox *>("channelMonitorAttribute"); }

	Device *m_dev = nullptr;
	ChannelMonitorView *m_view = nullptr;
};

void TST_ChannelMonitorView::init()
{
	m_dev = new Device();
	m_dev->setName("adc");
	auto add = [this](const char *id, bool out, const QStringList &attrs, const char *name = "") {
		auto *ch = new Channel(m_dev);
		ch->setId(id);
		ch->setName(name);
		ch->setIsOutput(out);
		for(const QString &a : attrs) {
			makeAttribute(ch, a, a == "scale" ? "0.5" : "10");
		}
	};
	add("voltage0", false, {"raw", "scale"});
	add("voltage1", false, {"raw", "scale"}, "vin");
	add("temp0", false, {"input"});
	add("voltage2", true, {"raw"});
	m_view = new ChannelMonitorView(m_dev);
}

void TST_ChannelMonitorView::cleanup()
{
	delete m_view;
	m_view = nullptr;
	delete m_dev;
	m_dev = nullptr;
}

void TST_ChannelMonitorView::onlyInputChannels()
{
	const QList<Channel *> chs = ChannelMonitorView::inputChannels(m_dev);
	QCOMPARE(chs.size(), 3);
	for(Channel *ch : chs) {
		QVERIFY(!ch->isOutput());
	}
}

void TST_ChannelMonitorView::defaultsToRaw()
{
	QCOMPARE(m_view->attributeName(), QStringLiteral("raw"));
	QCOMPARE(m_view->polledKeys(), QStringList({"voltage0", "voltage1"}));
	QCOMPARE(table()->rowCount(), 2);
	QCOMPARE(table()->item(0, 0)->text(), QStringLiteral("voltage0"));
	QCOMPARE(table()->item(1, 0)->text(), QStringLiteral("voltage1 (vin)"));
}

void TST_ChannelMonitorView::attributeComboListsNames()
{
	QComboBox *c = attrCombo();
	QVERIFY(c);
	QStringList items;
	for(int i = 0; i < c->count(); ++i) {
		items.append(c->itemText(i));
	}
	QCOMPARE(items, QStringList({"input", "raw", "scale"}));
	QCOMPARE(c->currentText(), QStringLiteral("raw"));
}

void TST_ChannelMonitorView::changingAttributeChangesRows()
{
	attrCombo()->setCurrentText("input");
	QCOMPARE(m_view->attributeName(), QStringLiteral("input"));
	QCOMPARE(m_view->polledKeys(), QStringList({"temp0"}));
	QCOMPARE(table()->rowCount(), 1);
	m_view->setAttributeName("raw");
	QCOMPARE(attrCombo()->currentText(), QStringLiteral("raw"));
	QCOMPARE(table()->rowCount(), 2);
}

void TST_ChannelMonitorView::pollsRawWithoutScale()
{
	QVERIFY(!m_view->monitor()->scaled());
	m_view->monitor()->start();
	QTRY_VERIFY(m_view->monitor()->stats("voltage0").count > 0);
	QCOMPARE(m_view->monitor()->stats("voltage0").value, 10.0); // not (10 * 0.5)
	QVERIFY(m_view->monitor()->samples("temp0").isEmpty());
	QVERIFY(m_view->monitor()->samples("voltage2").isEmpty());
	m_view->monitor()->stop();
}

void TST_ChannelMonitorView::noDeviceColumn()
{
	QCOMPARE(table()->columnCount(), 5);
	QCOMPARE(table()->horizontalHeaderItem(0)->text(), QStringLiteral("Channel"));
}

void TST_ChannelMonitorView::periodDefaultsToOneSecond() { QCOMPARE(m_view->monitor()->period(), 1000); }

void TST_ChannelMonitorView::contextHasNoChannels()
{
	QObject ctx;
	m_dev->setParent(&ctx);
	QVERIFY(ChannelMonitorView::inputChannels(&ctx).isEmpty());
	m_dev->setParent(nullptr);
}

QTEST_MAIN(TST_ChannelMonitorView)
#include "tst_channelmonitorview.moc"
