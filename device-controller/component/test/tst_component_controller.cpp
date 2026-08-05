#include "component/attribute.h"
#include "component/channel.h"
#include "component/context.h"
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

class TstComponentController : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void adoptEmitsAndBuildsTree();
	void refCountKeepsContextAlive();
	void disconnectCascades();
	void unknownBackendReturnsNull();
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
	// M2k has no registered factory -> connectCtx yields an empty handle.
	QVERIFY(!Controller::connectCtx("fake://m2k", BackendKind::M2k));
}

QTEST_MAIN(TstComponentController)
#include "tst_component_controller.moc"
