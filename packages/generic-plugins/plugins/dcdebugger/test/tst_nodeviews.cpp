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
#include "nodepanel.h"
#include "nodeviewregistry.h"

#include <QLabel>
#include <QTabWidget>
#include <QtTest>

#include <component/context.h>
#include <component/device.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

namespace {
// A view for devices whose name starts with `prefix`, as a plugin would register one.
class LabelViewFactory : public NodeViewFactory
{
public:
	LabelViewFactory(const QString &id, const QString &title, const QString &prefix = "dev")
		: m_id(id)
		, m_title(title)
		, m_prefix(prefix)
	{}
	QString id() const override { return m_id; }
	QString title() const override { return m_title; }
	bool supports(QObject *node, NodeKind kind) const override
	{
		return kind == NodeKind::Device && node->objectName().startsWith(m_prefix);
	}
	QWidget *create(QObject *, NodeKind, NodePanel *panel) override
	{
		auto *label = new QLabel(m_title, panel);
		label->setObjectName(m_id);
		return label;
	}

private:
	QString m_id;
	QString m_title;
	QString m_prefix;
};

QStringList ids(const QList<NodeViewFactory *> &factories)
{
	QStringList out;
	for(NodeViewFactory *f : factories) {
		out.append(f->id());
	}
	return out;
}

QStringList tabTitles(NodePanel *panel)
{
	QStringList out;
	if(auto *tabs = panel->findChild<QTabWidget *>()) {
		for(int i = 0; i < tabs->count(); ++i) {
			out.append(tabs->tabText(i));
		}
	}
	return out;
}
} // namespace

class TST_NodeViews : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void cleanup();
	void builtinViewsAreRegistered();
	void streamsGetTheirInstrument();
	void pluginViewIsShownInPriorityOrder();
	void sameIdReplacesTheView();
	void disabledViewIsNotShown();
	void unregisteredViewIsGone();

private:
	NodeViewRegistry *reg() const { return NodeViewRegistry::instance(); }
};

void TST_NodeViews::initTestCase() { Preferences::set("general_use_docking_if_available", false); }

void TST_NodeViews::cleanup()
{
	for(const QString &id : {"custom", "second"}) {
		reg()->unregisterFactory(id);
		Preferences::set(NodeViewRegistry::preferenceKey(id), true);
	}
	Preferences::set(NodeViewRegistry::preferenceKey("acquisition"), true);
	Preferences::set(NodeViewRegistry::preferenceKey("rawacquisition"), true);
}

void TST_NodeViews::builtinViewsAreRegistered()
{
	QCOMPARE(ids(reg()->factories()),
		 QStringList({"acquisition", "rawacquisition", "generator", "ping", "monitor", "channelmonitor",
			      "registers"}));
	for(NodeViewFactory *f : reg()->factories()) {
		QVERIFY(NodeViewRegistry::isEnabled(f->id()));
		QCOMPARE(Preferences::get(NodeViewRegistry::preferenceKey(f->id())).toBool(), true);
	}
	QCOMPARE(reg()->factory("monitor")->title(), QStringLiteral("Monitor"));
	QVERIFY(!reg()->factory("missing"));
}

void TST_NodeViews::streamsGetTheirInstrument()
{
	FakeInputStream in(1);
	FakeOutputStream out(1);
	QCOMPARE(ids(reg()->factoriesFor(&in, NodeKind::InputStream)), QStringList({"acquisition", "rawacquisition"}));
	QCOMPARE(ids(reg()->factoriesFor(&out, NodeKind::OutputStream)), QStringList({"generator"}));
	QVERIFY(reg()->factoriesFor(nullptr, NodeKind::Device).isEmpty());
}

void TST_NodeViews::pluginViewIsShownInPriorityOrder()
{
	Context ctx;
	auto *dev = new Device(&ctx);
	dev->setObjectName("dev0");
	reg()->registerFactory(std::make_unique<LabelViewFactory>("custom", "Custom"), 10);
	reg()->registerFactory(std::make_unique<LabelViewFactory>("second", "Second"), 200);
	QCOMPARE(ids(reg()->factoriesFor(dev, NodeKind::Device)), QStringList({"second", "custom"}));

	std::unique_ptr<NodePanel> panel(createNodePanel(dev, NodeKind::Device, "dev0", {}, {}));
	QVERIFY(panel->findChild<QLabel *>("custom"));
	QVERIFY(panel->findChild<QLabel *>("second"));
	// The views first, in registry order, then the attributes.
	QCOMPARE(tabTitles(panel.get()), QStringList({"Second", "Custom", "Attributes"}));

	auto *other = new Device(&ctx);
	other->setObjectName("adc");
	QVERIFY(reg()->factoriesFor(other, NodeKind::Device).isEmpty());
}

void TST_NodeViews::sameIdReplacesTheView()
{
	Context ctx;
	auto *dev = new Device(&ctx);
	dev->setObjectName("dev0");
	reg()->registerFactory(std::make_unique<LabelViewFactory>("custom", "Old"));
	reg()->registerFactory(std::make_unique<LabelViewFactory>("custom", "New"));
	QCOMPARE(reg()->factoriesFor(dev, NodeKind::Device).size(), 1);
	QCOMPARE(reg()->factory("custom")->title(), QStringLiteral("New"));
}

void TST_NodeViews::disabledViewIsNotShown()
{
	FakeInputStream in(1);
	Preferences::set(NodeViewRegistry::preferenceKey("acquisition"), false);
	Preferences::set(NodeViewRegistry::preferenceKey("rawacquisition"), false);
	QVERIFY(reg()->factoriesFor(&in, NodeKind::InputStream).isEmpty());
	std::unique_ptr<NodePanel> panel(createNodePanel(&in, NodeKind::InputStream, "in", {}, {}));
	QVERIFY(!panel->findChild<QTabWidget *>()); // no view: no tabs, only the sections
}

void TST_NodeViews::unregisteredViewIsGone()
{
	Context ctx;
	auto *dev = new Device(&ctx);
	dev->setObjectName("dev0");
	reg()->registerFactory(std::make_unique<LabelViewFactory>("custom", "Custom"));
	reg()->unregisterFactory("custom");
	QVERIFY(!reg()->factory("custom"));
	QVERIFY(reg()->factoriesFor(dev, NodeKind::Device).isEmpty());
}

QTEST_MAIN(TST_NodeViews)
#include "tst_nodeviews.moc"
