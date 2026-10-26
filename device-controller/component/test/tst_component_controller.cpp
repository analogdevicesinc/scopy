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

#include "component/attribute.h"
#include "component/channel.h"
#include "component/connection.h"
#include "component/connectionprovider.h"
#include "component/context.h"
#include "component/contextbackend.h"
#include "component/controller.h"
#include "component/device.h"
#include "core/pooledcmdexecutor.h"

#include <QPointer>
#include <QSignalSpy>
#include <QTest>

using namespace scopy;
using namespace scopy::component;

// Build a fake tree without hardware: Context → Device → Channel → Attribute.
static Context *makeFakeTree()
{
	auto *ctx = new Context;
	ctx->setName("fake-ctx");
	auto *dev = new Device(ctx);
	dev->setId("dev0");
	auto *ch = new Channel(dev);
	ch->setId("voltage0");
	auto *a1 = new Attribute(ch);
	a1->setName("frequency");
	auto *a2 = new Attribute(dev);
	a2->setName("name");
	return ctx;
}

class FakeConnection : public Connection
{
	Q_OBJECT
public:
	FakeConnection(QString uri, ConnectionKind kind, QString model)
		: Connection(std::move(uri), std::move(kind))
		, model(std::move(model))
	{}
	QString model;
	bool started = false;
	void start() override { started = true; }
};

// Transport serving "<kind>:<model>" uris.
class FakeProvider : public ConnectionProvider
{
public:
	explicit FakeProvider(ConnectionKind kind = "fake")
		: m_kind(std::move(kind))
	{}
	ConnectionKind kind() const override { return m_kind; }
	Connection *open(const QString &uri) override
	{
		if(!uri.startsWith(m_kind + ":")) {
			return nullptr;
		}
		return new FakeConnection(uri, m_kind, uri.section(':', 1));
	}
	QVector<ScanResult> scan(const QString &) override { return {{m_kind + ":0", "fake device", m_kind}}; }

private:
	ConnectionKind m_kind;
};

// View compatible with connections whose model matches (empty = any).
class FakeBackend : public ContextBackend
{
public:
	FakeBackend(BackendId id, ConnectionKind kind = "fake", QString model = {}, int threads = -1)
		: m_id(std::move(id))
		, m_kind(std::move(kind))
		, m_model(std::move(model))
		, m_threads(threads)
	{}
	BackendId id() const override { return m_id; }
	ConnectionKind connectionKind() const override { return m_kind; }
	bool compatible(const Connection &c) const override
	{
		auto *fc = c.as<FakeConnection>();
		return fc && (m_model.isEmpty() || fc->model == m_model);
	}
	int maxThreads() const override { return m_threads; }
	Context *create(Connection &) override { return makeFakeTree(); }
	void destroy(Context *) override { destroyed++; }
	int destroyed = 0;

private:
	BackendId m_id;
	ConnectionKind m_kind;
	QString m_model;
	int m_threads;
};

class TstComponentController : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void adoptEmitsAndBuildsTree();
	void refCountKeepsContextAlive();
	void disconnectCascades();
	void unknownBackendReturnsNull();
	void runtimeBackendBuildsTree();
	void duplicateRegistrationIsRefused();
	void unregisterRefusedWhileOpen();
	void openDecidesCompatibleBackends();
	void viewsShareOneConnection();
	void connectionHandleKeepsConnectionAlive();
	void serialBackendForcesSerialExecutor();
	void lateBackendJoinsOpenConnection();
	void otherKindOnOpenUriFails();
};

void TstComponentController::initTestCase()
{
	QVERIFY(Controller::GetInstance()->registerProvider(std::make_shared<FakeProvider>("fake")));
}

void TstComponentController::adoptEmitsAndBuildsTree()
{
	Controller *c = Controller::GetInstance();
	QSignalSpy opened(c, &Controller::connectionOpened);
	QSignalSpy created(c, &Controller::contextCreated);

	Context *tree = makeFakeTree();
	ContextHandle h = c->adopt("fake://adopt", tree);

	QVERIFY(h);
	QCOMPARE(opened.count(), 1);
	QCOMPARE(created.count(), 1);
	QCOMPARE(h.backend(), QString("adopted"));
	QCOMPARE(tree->connection(), h.connection());
	QCOMPARE(tree->backend(), QString("adopted"));
	QCOMPARE(tree->findChildren<Attribute *>().size(), 2);
	QCOMPARE(tree->findChildren<Channel *>().size(), 1);

	QVERIFY(!c->adopt("fake://adopt", makeFakeTree()));
}

void TstComponentController::refCountKeepsContextAlive()
{
	Controller *c = Controller::GetInstance();
	ContextHandle h1 = c->adopt("fake://ref", makeFakeTree());
	QVERIFY(h1);

	ContextHandle h2 = Controller::context("fake://ref", "adopted");
	QVERIFY(h2);
	QCOMPARE(h2.get(), h1.get());
	QVERIFY(!Controller::context("fake://ref", "iio"));

	QSignalSpy destroyed(c, &Controller::contextDestroyed);
	QSignalSpy closed(c, &Controller::connectionClosed);

	h2.reset();
	QCOMPARE(destroyed.count(), 0);

	h1.reset();
	QCOMPARE(destroyed.count(), 1);
	QCOMPARE(closed.count(), 1);

	QVERIFY(!Controller::context("fake://ref", "adopted"));
}

void TstComponentController::disconnectCascades()
{
	Controller *c = Controller::GetInstance();
	Context *ctx = makeFakeTree();
	QPointer<Attribute> attr = ctx->findChildren<Attribute *>().first();
	ContextHandle h = c->adopt("fake://cascade", ctx);
	QVERIFY(h);
	QPointer<Connection> conn = h.connection();

	h.reset();
	QTRY_VERIFY(attr.isNull());
	QTRY_VERIFY(conn.isNull());
}

void TstComponentController::unknownBackendReturnsNull()
{
	QVERIFY(!Controller::connectCtx("fake:m2k", "m2k"));
	QVERIFY(!Controller::open("fake:x", "nokind"));
}

void TstComponentController::runtimeBackendBuildsTree()
{
	Controller *c = Controller::GetInstance();
	QSignalSpy registered(c, &Controller::backendRegistered);
	auto b = std::make_shared<FakeBackend>("fake-rt");
	QVERIFY(c->registerBackend(b));
	QCOMPARE(registered.count(), 1);
	QVERIFY(c->backends().contains("fake-rt"));
	QVERIFY(c->providers().contains("fake"));
	QCOMPARE(c->scan("fake").size(), 1);

	{
		ContextHandle h = Controller::connectCtx("fake:rt", "fake-rt");
		QVERIFY(h);
		QCOMPARE(h->findChildren<Attribute *>().size(), 2);
		QCOMPARE(h->backend(), QString("fake-rt"));
		QVERIFY(h->executor() != nullptr);
		QCOMPARE(h->executor(), h.connection()->executor());
		QVERIFY(static_cast<FakeConnection *>(h.connection())->started);
	}
	QCOMPARE(b->destroyed, 1);
	QVERIFY(c->unregisterBackend("fake-rt"));
	QVERIFY(!c->backends().contains("fake-rt"));
}

void TstComponentController::duplicateRegistrationIsRefused()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-dup")));
	QVERIFY(!c->registerBackend(std::make_shared<FakeBackend>("fake-dup")));
	QVERIFY(!c->registerBackend(std::make_shared<FakeBackend>("")));
	QVERIFY(c->unregisterBackend("fake-dup"));
	QVERIFY(!c->registerProvider(std::make_shared<FakeProvider>("fake")));
	QVERIFY(!c->registerProvider(std::make_shared<FakeProvider>("")));
}

void TstComponentController::unregisterRefusedWhileOpen()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-open")));
	ContextHandle h = Controller::connectCtx("fake:open", "fake-open");
	QVERIFY(h);
	QVERIFY(!c->unregisterBackend("fake-open"));
	h.reset();
	QVERIFY(c->unregisterBackend("fake-open"));
}

void TstComponentController::openDecidesCompatibleBackends()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("any", "fake")));
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("only-m2k", "fake", "m2k")));

	ConnectionHandle conn = Controller::open("fake:pluto", "fake");
	QVERIFY(conn);
	QVERIFY(conn->backends().contains("any"));
	QVERIFY(!conn->backends().contains("only-m2k"));
	QVERIFY(!Controller::connectCtx("fake:pluto", "only-m2k"));
	QVERIFY(Controller::connectCtx("fake:pluto", "any"));

	ConnectionHandle m2k = Controller::open("fake:m2k", "fake");
	QVERIFY(m2k);
	QVERIFY(m2k->backends().contains("only-m2k"));
	QVERIFY(m2k->backends().contains("any"));

	conn.reset();
	m2k.reset();
	QVERIFY(c->unregisterBackend("any"));
	QVERIFY(c->unregisterBackend("only-m2k"));
}

void TstComponentController::viewsShareOneConnection()
{
	Controller *c = Controller::GetInstance();
	auto a = std::make_shared<FakeBackend>("view-a");
	auto b = std::make_shared<FakeBackend>("view-b");
	QVERIFY(c->registerBackend(a));
	QVERIFY(c->registerBackend(b));
	QSignalSpy opened(c, &Controller::connectionOpened);
	QSignalSpy closed(c, &Controller::connectionClosed);

	ContextHandle ha = Controller::connectCtx("fake:shared", "view-a");
	ContextHandle hb = Controller::connectCtx("fake:shared", "view-b");
	QVERIFY(ha);
	QVERIFY(hb);
	QCOMPARE(opened.count(), 1);
	QVERIFY(ha.get() != hb.get());
	QCOMPARE(ha.connection(), hb.connection());
	QCOMPARE(ha.connection()->contexts().size(), 2);
	QCOMPARE(ha.connection()->context("view-b"), hb.get());
	QCOMPARE(ha.connection()->backendOf(hb.get()), QString("view-b"));

	ContextHandle again = Controller::context("fake:shared", "view-a");
	QCOMPARE(again.get(), ha.get());

	// Dropping one view keeps the connection; the other view stays untouched.
	again.reset();
	ha.reset();
	QCOMPARE(a->destroyed, 1);
	QCOMPARE(closed.count(), 0);
	QCOMPARE(hb.connection()->contexts().size(), 1);

	hb.reset();
	QCOMPARE(b->destroyed, 1);
	QCOMPARE(closed.count(), 1);
	QVERIFY(c->unregisterBackend("view-a"));
	QVERIFY(c->unregisterBackend("view-b"));
}

void TstComponentController::connectionHandleKeepsConnectionAlive()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("keep")));
	QSignalSpy closed(c, &Controller::connectionClosed);

	ConnectionHandle conn = Controller::open("fake:keep", "fake");
	QVERIFY(conn);
	ContextHandle view = Controller::connectCtx("fake:keep", "keep");
	QVERIFY(view);
	QCOMPARE(view.connection(), conn.get());

	view.reset();
	QCOMPARE(closed.count(), 0);
	QVERIFY(!Controller::context("fake:keep", "keep"));

	conn.reset();
	QCOMPARE(closed.count(), 1);
	QVERIFY(c->unregisterBackend("keep"));
}

void TstComponentController::serialBackendForcesSerialExecutor()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("serial", "fake", QString(), 1)));
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("parallel", "fake", QString(), -1)));

	ConnectionHandle conn = Controller::open("fake:serial", "fake", 8);
	QVERIFY(conn);
	auto *exec = dynamic_cast<PooledCmdExecutor *>(conn->executor());
	QVERIFY(exec);
	QCOMPARE(exec->maxThreads(), 1);
	conn.reset();

	QVERIFY(c->unregisterBackend("serial"));
	ConnectionHandle par = Controller::open("fake:serial", "fake", 8);
	QVERIFY(par);
	QCOMPARE(dynamic_cast<PooledCmdExecutor *>(par->executor())->maxThreads(), 8);
	par.reset();
	QVERIFY(c->unregisterBackend("parallel"));
}

void TstComponentController::lateBackendJoinsOpenConnection()
{
	Controller *c = Controller::GetInstance();
	ConnectionHandle conn = Controller::open("fake:late", "fake", 4);
	QVERIFY(conn);
	QVERIFY(conn->backends().isEmpty());
	QSignalSpy changed(c, &Controller::backendsChanged);

	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("late")));
	QCOMPARE(changed.count(), 1);
	QVERIFY(conn->backends().contains("late"));
	QVERIFY(Controller::connectCtx("fake:late", "late"));

	// A serial-only backend cannot join a connection that already runs in parallel.
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("late-serial", "fake", QString(), 1)));
	QVERIFY(!conn->backends().contains("late-serial"));

	QVERIFY(c->unregisterBackend("late"));
	QCOMPARE(changed.count(), 2);
	QVERIFY(!conn->backends().contains("late"));
	conn.reset();
	QVERIFY(c->unregisterBackend("late-serial"));
}

void TstComponentController::otherKindOnOpenUriFails()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerProvider(std::make_shared<FakeProvider>("other")));
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("other-view", "other")));

	ConnectionHandle conn = Controller::open("fake:kind", "fake");
	QVERIFY(conn);
	QVERIFY(!Controller::open("fake:kind", "other"));
	QVERIFY(!Controller::connectCtx("fake:kind", "other-view"));
	conn.reset();
	QVERIFY(c->unregisterBackend("other-view"));
}

QTEST_MAIN(TstComponentController)
#include "tst_component_controller.moc"
