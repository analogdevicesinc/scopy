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
#include "component/context.h"
#include "component/contextbackend.h"
#include "component/controller.h"
#include "component/device.h"

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

// Runtime-registered backend that serves fake trees for "fake:" uris.
class FakeBackend : public ContextBackend
{
public:
	explicit FakeBackend(BackendId id = "fake")
		: m_id(std::move(id))
	{}
	BackendId id() const override { return m_id; }
	Context *create(const QString &uri, const int) override
	{
		Context *ctx = makeFakeTree();
		ctx->setUri(uri);
		return ctx;
	}
	QVector<ScanResult> scan(const QString &) override { return {{"fake:0", "fake device", m_id}}; }

private:
	BackendId m_id;
};

class TstComponentController : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void adoptEmitsAndBuildsTree();
	void refCountKeepsContextAlive();
	void disconnectCascades();
	void unknownBackendReturnsNull();
	void runtimeBackendBuildsTree();
	void duplicateBackendIsRefused();
	void unregisterRefusedWhileOpen();
	void otherBackendOnOpenUriFails();
};

void TstComponentController::adoptEmitsAndBuildsTree()
{
	Controller *c = Controller::GetInstance();
	QSignalSpy added(c, &Controller::componentAdded);

	Context *tree = makeFakeTree();
	ContextHandle h = c->adopt("fake://adopt", tree);

	QVERIFY(h);
	QCOMPARE(added.count(), 1);
	QCOMPARE(tree->findChildren<Attribute *>().size(), 2);
	QCOMPARE(tree->findChildren<Channel *>().size(), 1);

	// Re-adopting the same uri is refused (returns an empty handle).
	QVERIFY(!c->adopt("fake://adopt", makeFakeTree()));
}

void TstComponentController::refCountKeepsContextAlive()
{
	Controller *c = Controller::GetInstance();
	ContextHandle h1 = c->adopt("fake://ref", makeFakeTree());
	QVERIFY(h1);

	// A second handle to the same uri shares the context and bumps the count.
	ContextHandle h2 = Controller::context("fake://ref");
	QVERIFY(h2);
	QCOMPARE(h2.get(), h1.get());

	QSignalSpy removed(c, &Controller::componentRemoved);

	// Dropping one handle keeps the context alive (refCount 2 -> 1).
	h2.reset();
	QCOMPARE(removed.count(), 0);

	// Dropping the last handle tears it down (refCount 1 -> 0).
	h1.reset();
	QCOMPARE(removed.count(), 1);

	// No handles left: acquiring the uri now yields nothing.
	QVERIFY(!Controller::context("fake://ref"));
}

void TstComponentController::disconnectCascades()
{
	Controller *c = Controller::GetInstance();
	Context *ctx = makeFakeTree();
	QPointer<Attribute> attr = ctx->findChildren<Attribute *>().first();
	ContextHandle h = c->adopt("fake://cascade", ctx);
	QVERIFY(h);

	QSignalSpy removed(c, &Controller::componentRemoved);
	h.reset(); // RAII disconnect of the last handle

	QCOMPARE(removed.count(), 1);
	QTRY_VERIFY(attr.isNull()); // deleteLater cascades to the subtree
}

void TstComponentController::unknownBackendReturnsNull()
{
	int maxThreads = 1;
	// No "m2k" backend is registered -> connectCtx yields an empty handle.
	QVERIFY(!Controller::connectCtx("fake://m2k", "m2k", maxThreads));
}

void TstComponentController::runtimeBackendBuildsTree()
{
	Controller *c = Controller::GetInstance();
	QSignalSpy registered(c, &Controller::backendRegistered);
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-rt")));
	QCOMPARE(registered.count(), 1);
	QVERIFY(c->backends().contains("fake-rt"));
	QCOMPARE(c->scan("fake-rt").size(), 1);

	{
		ContextHandle h = Controller::connectCtx("fake:rt", "fake-rt");
		QVERIFY(h);
		QCOMPARE(h->findChildren<Attribute *>().size(), 2);
	}
	QVERIFY(c->unregisterBackend("fake-rt"));
	QVERIFY(!c->backends().contains("fake-rt"));
}

void TstComponentController::duplicateBackendIsRefused()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-dup")));
	QVERIFY(!c->registerBackend(std::make_shared<FakeBackend>("fake-dup")));
	QVERIFY(!c->registerBackend(std::make_shared<FakeBackend>("")));
	QVERIFY(c->unregisterBackend("fake-dup"));
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

void TstComponentController::otherBackendOnOpenUriFails()
{
	Controller *c = Controller::GetInstance();
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-a")));
	QVERIFY(c->registerBackend(std::make_shared<FakeBackend>("fake-b")));
	ContextHandle h = Controller::connectCtx("fake:shared", "fake-a");
	QVERIFY(h);
	QVERIFY(!Controller::connectCtx("fake:shared", "fake-b"));
	// Same backend or no backend shares the open context.
	ContextHandle same = Controller::connectCtx("fake:shared", "fake-a");
	ContextHandle any = Controller::connectCtx("fake:shared");
	QCOMPARE(same.get(), h.get());
	QCOMPARE(any.get(), h.get());
	same.reset();
	any.reset();
	h.reset();
	QVERIFY(c->unregisterBackend("fake-a"));
	QVERIFY(c->unregisterBackend("fake-b"));
}

QTEST_MAIN(TstComponentController)
#include "tst_component_controller.moc"
