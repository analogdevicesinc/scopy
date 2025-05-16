/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 */

#include <QTest>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <customSourceBlocks.h>
#include <customFilterBlocks.h>
#include <blockManager.h>

using namespace scopy::iiosink;

class TST_BlockManager : public QObject
{
	Q_OBJECT

private:
	int m_test_success = 1;
	int counter = 0;
	int counter2 = 0;

private Q_SLOTS:
	void testAddFilter1();
	void testAddFilter2();
	void testAddFilter3();
	void testAddFilter4_data();
	void testAddFilter4();
	void testAddFilter5_data();
	void testAddFilter5();
};

void TST_BlockManager::testAddFilter1()
{
	// add 2 channels
	FileSourceBlock *fileSource = new FileSourceBlock("test.csv");
	fileSource->enChannel(true, 0);
	fileSource->enChannel(true, 1);
	AddFilterBlock *filter = new AddFilterBlock();
	int buffer_size = 100;
	m_test_success = 1;

	BlockManager::connectBlockToFilter(fileSource, 0, 0, filter);
	BlockManager::connectBlockToFilter(fileSource, 1, 1, filter);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;

	connect(
		filter, &AddFilterBlock::newData, this,
		[=, this, &loop](ChannelData *data, uint ch) {
			m_test_success = 1;
			for(int i = 0; i < data->data.size(); i++) {
				if(data->data[i] != i + i * 10) {
					m_test_success = 0;
					break;
				}
			}
			loop.quit();
		},
		Qt::QueuedConnection);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	timer.start(2000); // timeout in ms
	fileSource->setBufferSize(buffer_size);
	Q_EMIT fileSource->requestData();
	loop.exec();

	QVERIFY(m_test_success);
}

void TST_BlockManager::testAddFilter2()
{
	// only 1 channel is enabled
	// expect no out signal

	// add 2 channels
	FileSourceBlock *fileSource = new FileSourceBlock("test.csv");
	fileSource->enChannel(true, 1);
	AddFilterBlock *filter = new AddFilterBlock();
	int buffer_size = 100;
	m_test_success = 1;

	BlockManager::connectBlockToFilter(fileSource, 0, 0, filter);
	BlockManager::connectBlockToFilter(fileSource, 1, 1, filter);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;

	connect(
		filter, &AddFilterBlock::newData, this,
		[=, this, &loop](ChannelData *data, uint ch) {
			m_test_success = 0;
			loop.quit();
		},
		Qt::QueuedConnection);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	timer.start(2000); // timeout in ms
	fileSource->setBufferSize(buffer_size);
	Q_EMIT fileSource->requestData();
	loop.exec();

	QVERIFY(m_test_success);
}

void TST_BlockManager::testAddFilter3()
{
	// only 1 channel is connected
	// add 2 channels
	FileSourceBlock *fileSource = new FileSourceBlock("test.csv");
	fileSource->enChannel(true, 0);
	fileSource->enChannel(true, 1);
	AddFilterBlock *filter = new AddFilterBlock();
	int buffer_size = 100;
	m_test_success = 1;

	BlockManager::connectBlockToFilter(fileSource, 0, 0, filter);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;

	connect(
		filter, &AddFilterBlock::newData, this,
		[=, this, &loop](ChannelData *data, uint ch) {
			m_test_success = 1;
			for(int i = 0; i < data->data.size(); i++) {
				if(data->data[i] != i) {
					m_test_success = 0;
					break;
				}
			}
			loop.quit();
		},
		Qt::QueuedConnection);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	timer.start(2000); // timeout in ms
	fileSource->setBufferSize(buffer_size);
	Q_EMIT fileSource->requestData();
	loop.exec();

	QVERIFY(m_test_success);
}

void TST_BlockManager::testAddFilter4_data()
{
	QTest::addColumn<bool>("copy");
	QTest::addColumn<bool>("threaded");

	QTest::newRow("0_0") << false << false;
	QTest::newRow("0_1") << false << true;
	QTest::newRow("1_0") << true << false;
	QTest::newRow("1_1") << true << true;
}

void TST_BlockManager::testAddFilter4()
{
	QFETCH(bool, copy);
	QFETCH(bool, threaded);

	// cyclic run
	// add 2 channels
	FileSourceBlock *fileSource = new FileSourceBlock("test.csv");
	fileSource->enChannel(true, 0);
	fileSource->enChannel(true, 1);
	AddFilterBlock *filter = new AddFilterBlock();
	filter->doCopy(copy);

	int buffer_size = 100;
	m_test_success = 1;

	BlockManager::connectBlockToFilter(fileSource, 0, 0, filter);
	BlockManager::connectBlockToFilter(fileSource, 1, 1, filter);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	counter = 0;

	BlockManager *manager = new BlockManager();
	manager->addLink(fileSource, 0, filter, 0, 0, threaded);
	manager->addLink(fileSource, 1, filter, 0, 0, threaded);

	connect(
		manager, &BlockManager::newData, this,
		[this](ChannelData *data, uint ch) {
			counter++;
			for(int i = 0; i < data->data.size(); i++) {
				if(data->data[i] != i + i * 10) {
					m_test_success = 0;
					return;
				}
			}
		},
		Qt::QueuedConnection);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	int timeout = 10;
	timer.start(timeout); // timeout in ms
	fileSource->setBufferSize(buffer_size);
	manager->start(true);
	loop.exec();
	manager->stop();

	QVERIFY(m_test_success);
	QVERIFY(counter > 10);
}

void TST_BlockManager::testAddFilter5_data()
{
	QTest::addColumn<bool>("copy");
	QTest::addColumn<bool>("threaded");
	QTest::addColumn<bool>("waitForAllSources");

	QTest::newRow("000") << false << false << false;
	QTest::newRow("010") << false << true << false;
	QTest::newRow("011") << false << true << true;
	QTest::newRow("100") << true << false << false;
	QTest::newRow("101") << true << false << true;
	QTest::newRow("110") << true << true << false;
	QTest::newRow("111") << true << true << true;
}

void TST_BlockManager::testAddFilter5()
{
	QFETCH(bool, copy);
	QFETCH(bool, threaded);
	QFETCH(bool, waitForAllSources);

	// cyclic run
	// 2 sources

	FileSourceBlock *fileSource1 = new FileSourceBlock("test.csv");
	FileSourceBlock *fileSource2 = new FileSourceBlock("test.csv");
	fileSource1->enChannel(true, 0);
	fileSource2->enChannel(true, 0);
	AddFilterBlock *filter1 = new AddFilterBlock();
	AddFilterBlock *filter2 = new AddFilterBlock();
	AddFilterBlock *filter3 = new AddFilterBlock();
	filter1->doCopy(copy);
	filter2->doCopy(copy);
	filter3->doCopy(copy);

	int buffer_size = 10;
	m_test_success = 1;

	BlockManager::connectBlockToFilter(fileSource1, 0, 0, filter1);
	BlockManager::connectBlockToFilter(fileSource2, 0, 0, filter2);
	BlockManager::connectBlockToFilter(filter2, 0, 0, filter3);

	QTimer timer;
	timer.setSingleShot(true);
	QEventLoop loop;
	counter = 0;
	counter2 = 0;

	BlockManager *manager = new BlockManager(waitForAllSources);
	manager->addLink(fileSource1, 0, filter1, 0, 0, threaded);
	manager->addLink(fileSource2, 0, filter3, 0, 1, threaded);

	connect(
		manager, &BlockManager::newData, this,
		[this](ChannelData *data, uint ch) {
			if(ch != 0)
				return;

			counter++;
		},
		Qt::QueuedConnection);
	connect(
		manager, &BlockManager::newData, this,
		[this](ChannelData *data, uint ch) {
			if(ch != 1)
				return;

			counter2++;
		},
		Qt::QueuedConnection);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit, Qt::QueuedConnection);

	int timeout = 100;
	timer.start(timeout); // timeout in ms
	fileSource1->setBufferSize(buffer_size);
	fileSource2->setBufferSize(buffer_size);
	manager->start(true);
	loop.exec();
	manager->stop();

	bool wait_success = true;
	if(waitForAllSources) {
		wait_success = std::abs(counter2 - counter) < 3;
	}

	QVERIFY(m_test_success);
	QVERIFY(counter > 50);
	QVERIFY(wait_success);
}

QTEST_MAIN(TST_BlockManager)
#include "tst_blockmanager.moc"
