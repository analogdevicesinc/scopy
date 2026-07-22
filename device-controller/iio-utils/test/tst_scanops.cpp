#include "backendfixture.h"

#include "iioutil/iscanops.h"

using namespace scopy;
using namespace scopy::iio;

class TestScanOps : public QObject
{
	Q_OBJECT
private slots:
	// scan() must be callable on every backend and return a (possibly empty)
	// list without crashing — no live context needed.
	void scanDoesNotCrash();
	void scanDoesNotCrash_data() { dctest::addBackendRows(); }
};

void TestScanOps::scanDoesNotCrash()
{
	DC_REQUIRE_BACKEND();
	const QVector<ScanResult> results = b->scanOps()->scan();
	// Each result, if any, must carry a non-empty URI.
	for(const ScanResult &r : results) {
		QVERIFY(!r.uri.isEmpty());
	}
}

QTEST_MAIN(TestScanOps)
#include "tst_scanops.moc"
