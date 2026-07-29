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
	void adoptAndFindAll();
	void disconnectCascades();
	void unknownBackendReturnsNull();
};

void TstComponentController::adoptAndFindAll()
{
	Controller c;
	QSignalSpy added(&c, &Controller::componentAdded);
	c.adopt(makeFakeTree());

	QCOMPARE(added.count(), 1);
	QCOMPARE(c.contexts().size(), 1);
	QCOMPARE(c.findAll<Attribute>().size(), 2);
	QCOMPARE(c.findAll<Channel>().size(), 1);
}

void TstComponentController::disconnectCascades()
{
	Controller c;
	Context *ctx = makeFakeTree();
	QPointer<Attribute> attr = ctx->findChildren<Attribute *>().first();
	c.adopt(ctx);

	QSignalSpy removed(&c, &Controller::componentRemoved);
	c.disconnect(ctx);

	QCOMPARE(removed.count(), 1);
	QCOMPARE(c.contexts().size(), 0);
	QTRY_VERIFY(attr.isNull()); // deleteLater cascades to the subtree
}

void TstComponentController::unknownBackendReturnsNull()
{
	Controller c;
	QCOMPARE(c.connect("uri", BackendKind::M2k), nullptr);
}

QTEST_MAIN(TstComponentController)
#include "tst_component_controller.moc"
