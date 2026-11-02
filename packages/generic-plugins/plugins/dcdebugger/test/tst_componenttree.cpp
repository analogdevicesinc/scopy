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

#include "componenttreemodel.h"

#include <QtTest>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/connection.h>
#include <component/context.h>
#include <component/device.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;

class TST_ComponentTree : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void rootIsConnection();
	void childrenAreGrouped();
	void groupsAreOrdered();
	void pathSkipsGroups();
	void attributeValueFollowsCache();
	void childObjectsWalkGroups();

private:
	QModelIndex child(const QModelIndex &parent, const QString &name) const;

	Connection *m_conn = nullptr;
	Device *m_dev = nullptr;
	Attribute *m_attr = nullptr;
	ComponentTreeModel *m_model = nullptr;
};

void TST_ComponentTree::init()
{
	m_conn = new Connection("test:0", "test");
	auto *ctx = new Context();
	ctx->setName("ctx");
	ctx->setBackend("iio");
	m_conn->add("iio", ctx);

	auto *hw = new Attribute(ctx);
	hw->setName("hw_model");

	m_dev = new Device(ctx);
	m_dev->setId("iio:device0");
	m_dev->setName("dev0");
	m_attr = new Attribute(m_dev);
	m_attr->setName("sampling_frequency");
	m_attr->setUnit("Hz");
	(new Attribute(m_dev))->setName("calibscale");
	for(const QString &id : {"voltage0", "voltage1"}) {
		auto *ch = new Channel(m_dev);
		ch->setId(id);
		(new Attribute(ch))->setName("raw");
	}
	// A leaf with no generic base is listed under its class name.
	(new QObject(m_dev))->setObjectName("leaf");

	m_model = new ComponentTreeModel();
	m_model->setConnection(m_conn);
}

void TST_ComponentTree::cleanup()
{
	delete m_model;
	delete m_conn;
}

QModelIndex TST_ComponentTree::child(const QModelIndex &parent, const QString &name) const
{
	for(int r = 0; r < m_model->rowCount(parent); ++r) {
		const QModelIndex idx = m_model->index(r, ComponentTreeModel::NameColumn, parent);
		if(idx.data().toString() == name) {
			return idx;
		}
	}
	return {};
}

void TST_ComponentTree::rootIsConnection()
{
	QCOMPARE(m_model->rowCount(), 1);
	const QModelIndex root = m_model->index(0, 0);
	QCOMPARE(root.data().toString(), QString("test:0"));
	QCOMPARE(root.siblingAtColumn(ComponentTreeModel::ValueColumn).data().toString(), QString("test"));
	QCOMPARE(root.siblingAtColumn(ComponentTreeModel::InfoColumn).data().toString(), QString("Connection"));
	QCOMPARE(m_model->kindAt(root), NodeKind::Connection);
	QCOMPARE(m_model->objectAt(root), m_conn);
}

void TST_ComponentTree::childrenAreGrouped()
{
	const QModelIndex views = child(m_model->index(0, 0), "Views (1)");
	QVERIFY(views.isValid());
	QVERIFY(m_model->isGroup(views));
	const QModelIndex ctx = child(views, "iio");
	QVERIFY(ctx.isValid());
	QCOMPARE(m_model->kindAt(ctx), NodeKind::Context);

	const QModelIndex dev = m_model->indexOf(m_dev);
	QVERIFY(dev.isValid());
	QVERIFY(child(dev, "Attributes (2)").isValid());
	QVERIFY(child(dev, "Channels (2)").isValid());
	QVERIFY(child(dev, "QObject (1)").isValid());
	const QModelIndex ch = child(child(dev, "Channels (2)"), "voltage0");
	QCOMPARE(ch.siblingAtColumn(ComponentTreeModel::InfoColumn).data().toString(), QString("Channel (input)"));
	QVERIFY(child(ch, "Attributes (1)").isValid());
}

void TST_ComponentTree::groupsAreOrdered()
{
	const QModelIndex dev = m_model->indexOf(m_dev);
	QCOMPARE(m_model->index(0, 0, dev).data(ComponentTreeModel::GroupNameRole).toString(), QString("Attributes"));
	QCOMPARE(m_model->index(1, 0, dev).data(ComponentTreeModel::GroupNameRole).toString(), QString("Channels"));
	QCOMPARE(m_model->index(2, 0, dev).data(ComponentTreeModel::GroupNameRole).toString(), QString("QObject"));
}

void TST_ComponentTree::pathSkipsGroups()
{
	const QModelIndex attr = m_model->indexOf(m_attr);
	const auto path = m_model->pathOf(attr);
	QStringList names;
	for(const auto &s : path) {
		names.append(s.first);
	}
	QCOMPARE(names, QStringList({"test:0", "iio", "dev0", "sampling_frequency"}));
	QCOMPARE(path.first().second, m_conn);
	QCOMPARE(path.last().second, m_attr);
	QCOMPARE(attr.siblingAtColumn(ComponentTreeModel::InfoColumn).data().toString(), QString("Attribute (ro)"));
}

void TST_ComponentTree::attributeValueFollowsCache()
{
	m_attr->setCachedValue("1000000");
	const QModelIndex value = m_model->indexOf(m_attr).siblingAtColumn(ComponentTreeModel::ValueColumn);
	QCOMPARE(value.data().toString(), QString("1000000 Hz"));
}

void TST_ComponentTree::childObjectsWalkGroups()
{
	const QModelIndex dev = m_model->indexOf(m_dev);
	QCOMPARE(m_model->childObjects(dev).size(), 5);
	QCOMPARE(m_model->childObjects(child(dev, "Channels (2)")).size(), 2);
}

QTEST_GUILESS_MAIN(TST_ComponentTree)
#include "tst_componenttree.moc"
