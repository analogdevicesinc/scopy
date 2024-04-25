#include <QList>
#include <QTest>
#include "qpluginloader.h"
#include <pluginbase/plugin.h>
#include <dataloggerplugin/dataacquisitionmanager.hpp>
#include <dataloggerplugin/datamonitor/readabledatamonitormodel.hpp>
#include <dataloggerplugin/datamonitor/readstrategy/testreadstrategy.hpp>

using namespace scopy;
using namespace datamonitor;

class TST_DataMonitor : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void addMonitor();
	void removeMonitor();
	void activateMonitor();
	void disableMonitor();
	void disableOneOfManyUsers();
	void disabledChannelRead();
	void readData();
	void clearData();
};

void TST_DataMonitor::addMonitor()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();

	QVERIFY(dataAcquisitionManager->getDataMonitorMap()->isEmpty());

	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);

	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	QVERIFY(!dataAcquisitionManager->getDataMonitorMap()->isEmpty());
}

void TST_DataMonitor::removeMonitor()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();

	QVERIFY(dataAcquisitionManager->getDataMonitorMap()->isEmpty());

	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);

	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	QVERIFY(!dataAcquisitionManager->getDataMonitorMap()->isEmpty());

	dataAcquisitionManager->getDataMonitorMap()->remove(channelModel->getName());

	QVERIFY(dataAcquisitionManager->getDataMonitorMap()->isEmpty());
}

void TST_DataMonitor::activateMonitor()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	QVERIFY(dataAcquisitionManager->getActiveMonitors().isEmpty());

	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());

	QVERIFY(!dataAcquisitionManager->getActiveMonitors().isEmpty());
}

void TST_DataMonitor::disableMonitor()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());
	dataAcquisitionManager->updateActiveMonitors(false, channelModel->getName());

	QVERIFY(dataAcquisitionManager->getActiveMonitors().isEmpty());
}

void TST_DataMonitor::disableOneOfManyUsers()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	// simulate 2 instances using the monitor
	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());
	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());
	// one of the instances no longer uses the monitor
	dataAcquisitionManager->updateActiveMonitors(false, channelModel->getName());

	QVERIFY(!dataAcquisitionManager->getActiveMonitors().isEmpty());
}

void TST_DataMonitor::disabledChannelRead()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	channelModel->setReadStrategy(new TestReadStrategy());
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());

	dataAcquisitionManager->readData();
	dataAcquisitionManager->clearMonitorsData();

	dataAcquisitionManager->updateActiveMonitors(false, channelModel->getName());
	dataAcquisitionManager->readData();

	QVERIFY(channelModel->getYdata()->isEmpty());
}

void TST_DataMonitor::readData()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	channelModel->setReadStrategy(new TestReadStrategy());
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());

	dataAcquisitionManager->readData();

	QVERIFY(!channelModel->getYdata()->isEmpty());
}

void TST_DataMonitor::clearData()
{
	DataAcquisitionManager *dataAcquisitionManager = new DataAcquisitionManager();
	UnitOfMeasurement *um = new UnitOfMeasurement("Volt", "V");
	ReadableDataMonitorModel *channelModel = new ReadableDataMonitorModel("dev0:test ", "#FFFFFF", um);
	channelModel->setReadStrategy(new TestReadStrategy());
	dataAcquisitionManager->getDataMonitorMap()->insert(channelModel->getName(), channelModel);

	dataAcquisitionManager->updateActiveMonitors(true, channelModel->getName());

	dataAcquisitionManager->readData();
	dataAcquisitionManager->clearMonitorsData();

	QVERIFY(channelModel->getYdata()->isEmpty());
}

QTEST_MAIN(TST_DataMonitor)
#include "tst_datamonitor.moc"
