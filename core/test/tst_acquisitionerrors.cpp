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

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/sourceblock.h>

#include <QTest>

#include <stdexcept>

using namespace scopy::acq;

// What a source's failure looks like by the time it reaches the engine's error signal,
// which is what every tool's log shows.
class TST_AcquisitionErrors : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void startFailureNamesSourcePhaseAndCause();
	void steadyFailureIsReportedOnce();
};

namespace {

struct Report
{
	int severity;
	QString id;
	QString message;
};

class FailingStartSource : public SourceBlock
{
public:
	using SourceBlock::SourceBlock;
	void onStart() override
	{
		SourceBlock::onStart();
		throw std::runtime_error("writing voltage0/trigger_mode = always failed: unknown trigger option");
	}
	void acquire(DataStore *) override {}
};

class FailingReadSource : public SourceBlock
{
public:
	using SourceBlock::SourceBlock;
	void acquire(DataStore *) override
	{
		reportOnce(AcquisitionError::Severity::Warning, QStringLiteral("reading samples failed: timed out"));
	}
};

// Queued onto this thread: the engine emits from its worker
QList<Report> *collect(AcquisitionEngine &engine)
{
	auto *out = new QList<Report>();
	QObject::connect(
		&engine, &AcquisitionEngine::error, &engine,
		[out](int sev, const QString &id, const QString &msg) {
			out->append({sev, id, msg});
		},
		Qt::QueuedConnection);
	return out;
}

} // namespace

void TST_AcquisitionErrors::startFailureNamesSourcePhaseAndCause()
{
	DataStore store;
	AcquisitionEngine engine(&store);
	auto *src = new FailingStartSource(QStringLiteral("m2k"), &engine);
	engine.addSource(src);
	std::unique_ptr<QList<Report>> reports(collect(engine));

	engine.run();
	QTRY_VERIFY_WITH_TIMEOUT(!reports->isEmpty() && !engine.isRunning(), 5000);

	QCOMPARE(reports->size(), 1);
	QCOMPARE(reports->at(0).severity, static_cast<int>(AcquisitionError::Severity::Critical));
	QCOMPARE(reports->at(0).id, QStringLiteral("m2k"));
	QCOMPARE(reports->at(0).message,
		 QStringLiteral("onStart(): writing voltage0/trigger_mode = always failed: unknown trigger option"));
}

void TST_AcquisitionErrors::steadyFailureIsReportedOnce()
{
	DataStore store;
	AcquisitionEngine engine(&store);
	auto *src = new FailingReadSource(QStringLiteral("dc"), &engine);
	engine.addSource(src);
	std::unique_ptr<QList<Report>> reports(collect(engine));

	engine.run();
	QTest::qWait(200);
	engine.stop();
	QTRY_VERIFY_WITH_TIMEOUT(!engine.isRunning(), 5000);
	QCoreApplication::processEvents();

	// The engine adds its own "wrote no data" note; the source's failure is there once
	int count = 0;
	for(const Report &r : std::as_const(*reports)) {
		if(r.message == QStringLiteral("reading samples failed: timed out")) {
			QCOMPARE(r.severity, static_cast<int>(AcquisitionError::Severity::Warning));
			QCOMPARE(r.id, QStringLiteral("dc"));
			++count;
		}
	}
	QCOMPARE(count, 1);
}

QTEST_MAIN(TST_AcquisitionErrors)
#include "tst_acquisitionerrors.moc"
