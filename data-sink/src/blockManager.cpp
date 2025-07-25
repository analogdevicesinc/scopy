#include <include/data-sink/blockManager.h>
#include <iostream>
#include <QMetaType>

Q_LOGGING_CATEGORY(CAT_BLOCKMANAGER, "BLOCKMANAGER");
using namespace scopy::datasink;
Q_DECLARE_METATYPE(ChannelDataVector);

SourceBlockLink::SourceBlockLink(SourceBlock *sourceBlock, bool threaded)
	: QObject()
	, source(sourceBlock)
{
	finished_outputs = 0;
	active_outputs = 0;

	if(threaded) {
		thread = new QThread();
		thread->start();
		sourceBlock->moveToThread(thread);
	} else {
		thread = nullptr;
	}

	// connect(source, &SourceBlock::toggledCh, this, &SourceBlockLink::updateActiveOutputs, Qt::QueuedConnection);
}

SourceBlockLink::~SourceBlockLink() {}

uint SourceBlockLink::enabledSourceChCount()
{
	QSet<uint> counter;
	for(auto ch_set : connected_outputs) {
		for(auto ch : ch_set) {
			if(source->isChannelEn(ch)) {
				counter.insert(ch);
			}
		}
	}

	return counter.size();
}

void SourceBlockLink::addSource(uint out_ch, uint source_ch)
{
	if(connected_outputs.contains(out_ch)) {
		connected_outputs[out_ch].insert(source_ch);
	} else {
		QSet<uint> source_list;
		source_list.insert(source_ch);
		connected_outputs.insert(out_ch, source_list);
	}

	updateActiveOutputs();
}

void SourceBlockLink::removeSource(uint out_ch, uint source_ch)
{
	if(connected_outputs.contains(out_ch)) {
		connected_outputs[out_ch].remove(source_ch);
		if(connected_outputs[out_ch].empty()) {
			connected_outputs.remove(out_ch);
		}
	}
}

void SourceBlockLink::updateActiveOutputs()
{
	active_outputs = connected_outputs.count();
	for(auto ch_set : connected_outputs) {
		for(uint ch : ch_set)
			if(!source->isChannelEn(ch)) {
				active_outputs--;
				break;
			}
	}
}

BlockManager::BlockManager(QString name, bool waitforAllSources)
	: QObject()
	, m_waitforAllSources(waitforAllSources)
	, m_fps(0)
	, m_fpsTimer(new QElapsedTimer())
	, m_fpsTimeElapsed(0)
	, m_thread(new QThread())
	, m_globalBufferSize(0)
	, m_globalPlotSize(0)
	, m_name(name)
	, m_aqcCounter(0)
{
	moveToThread(m_thread);
	m_thread->start();

	connect(
		this, &BlockManager::sentAllData, this,
		[this]() {
			if(m_running && m_fps != 0) {
				// std::cout << "time elapsed: " << m_fpsTimer->elapsed() - m_fpsTimeElapsed << "     ";
				// std::cout << "time target: " << 1000 / m_fps << "     ";
				auto ms_to_wait = std::max((long long)0, (1000 / m_fps) - (m_fpsTimer->elapsed() - m_fpsTimeElapsed));
				// std::cout << ms_to_wait << std::endl;
				QThread::msleep(ms_to_wait);
			}
			m_fpsTimer->restart();
		},
		Qt::QueuedConnection);
};

void BlockManager::emitOutputData(SourceBlock *source)
{
	// wait for all sources to finish
	if(m_waitforAllSources) {
		bool do_request = true;

		// check if all sources are ready
		for(auto link : m_blockLinks) {
			if(link->finished_outputs < link->active_outputs && link->active_outputs > 0) {
				do_request = false;
				break;
			}
		}

		// request data to all sources and reset counters
		if(do_request) {
			Q_EMIT sentBufferData();

			// if(m_aqcCounter > 1) {
				m_aqcCounter--;
				if(m_aqcCounter == 0) {
					if(m_singleShot) {
						m_running = false;
						Q_EMIT requestStop();
					} else {
						refilAqcCounter();
					}
					Q_EMIT sentAllData();
				}
			// }

			for(auto link : m_blockLinks) {
				link->finished_outputs = 0;

				if(m_running && link->active_outputs > 0)
					onRequestData(m_blockLinks.key(link));
					// Q_EMIT m_blockLinks.key(link)->requestData();
			}

		}

	} else {
		if(source) {
			// request data for each source when finished
			if(m_blockLinks[source]->finished_outputs >= m_blockLinks[source]->active_outputs &&
			   m_blockLinks[source]->active_outputs > 0) {
				m_blockLinks[source]->finished_outputs = 0;

				if(!m_singleShot && m_running)
					onRequestData(source);
					// Q_EMIT source->requestData();

				Q_EMIT sentBufferData();
				Q_EMIT sentAllData();
				if(m_singleShot)
					Q_EMIT requestStop();
			}
		}
	}
}

BlockManager::~BlockManager()
{
	m_thread->exit();
};

void BlockManager::onRequestData(SourceBlock *source)
{
	// m_aqcCounter = (m_globalPlotSize + m_globalBufferSize - 1) / m_globalBufferSize;
	// int tmpAqcCounter = m_aqcCounter;
	// while(tmpAqcCounter > 0) {
	// 	tmpAqcCounter--;
	// if(m_aqcCounter > 0)
	// 	m_aqcCounter--;

	// for(int i = 0; i < m_aqcCounter; i++)
		Q_EMIT source->requestData();
	// }
}

int BlockManager::addOutputLink(SourceBlock *source, BasicBlock *final, uint final_ch, uint out_ch)
{
	if(m_requestDataConnections.contains(out_ch)) {
		if(!(m_requestDataConnections[out_ch]->final_block == final &&
		     m_requestDataConnections[out_ch]->final_block_ch == final_ch)) {
			qWarning(CAT_BLOCKMANAGER)
				<< "Failted to connect output. Channel " << out_ch
				<< " already has a link: " << final->getName() << " on channel " << final_ch;
		}
		return -1;
	}

	QMetaObject::Connection conn = connect(
		final, &BasicBlock::newData, this,
		[=](ChannelDataVector data, uint block_ch) {
			if(block_ch != final_ch)
				return;
			// std::cout << "------ sent newData: " << data.data.size() << "  on ch: " << out_ch << "  ss: " << m_runningSingleShot << std::endl;

			if(!m_running && !m_singleShot)
				return;


			std::cout << "------ actially sent newData: " << data.data.size() << "  on ch: " << out_ch << std::endl;
			Q_EMIT newData(data, out_ch);

			// if(--m_aqcCounter > 0)
			// 	return;

			m_blockLinks[source]->finished_outputs++;

			emitOutputData(source);
		},
		Qt::DirectConnection);
	m_requestDataConnections.insert(out_ch, new OutputConnection(final, final_ch, conn));

	return out_ch;
}

void BlockManager::removeLink(SourceBlock *source, uint source_ch, uint out_ch)
{
	if(!m_requestDataConnections.contains(out_ch)) {
		qWarning(CAT_BLOCKMANAGER) << "Failted to remove output. Channel " << out_ch << " doesn't exist";
		return;
	}

	delete m_requestDataConnections.value(out_ch);
	m_requestDataConnections.remove(out_ch);

	m_blockLinks[source]->removeSource(out_ch, source_ch);
	// remove link if it has no outputs remaining
	if(m_blockLinks[source]->connected_outputs.empty()) {
		delete m_blockLinks[source];
		m_blockLinks.remove(source);
	}
}

// out_ch is unique for every link
void BlockManager::addLink(SourceBlock *source, uint source_ch, BasicBlock *final, uint final_ch, uint out_ch,
			   bool threaded)
{
	// add new link to list
	if(m_blockLinks.contains(source)) {
		m_blockLinks[source]->addSource(out_ch, source_ch);


	} else {
		SourceBlockLink *link = new SourceBlockLink(source, threaded);
		link->addSource(out_ch, source_ch);
		m_blockLinks.insert(source, link);

		connect(
			source, &SourceBlock::toggledCh, this,
			[=](uint ch, bool en) {
				m_blockLinks[source]->updateActiveOutputs();

				// if running and a ch from a source with no enabled channels is enabled, start the
				// request cycle again
				if(!m_singleShot && m_running && en && m_blockLinks[source]->enabledSourceChCount() == 1) {
					onRequestData(source);
					// Q_EMIT source->requestData();
				}
			},
			Qt::QueuedConnection);


		connect(this, &BlockManager::sentAllData, source, &SourceBlock::resetPlotBuffer);
		// if(source->bufferSize() == 0) {
			// source->setBufferSize(m_globalBufferSize);
		// }

		// if(source->plotSize() == 0) {
			// source->setPlotSize(m_globalPlotSize);
		// }
	}

	source->setBufferSize(m_globalBufferSize);
	source->setPlotSize(m_globalPlotSize);

	// if input already exists, the threaded status cannot be changed
	if(m_blockLinks[source]->thread) {
		final->moveToThread(m_blockLinks[source]->thread);
	}

	// connect final block to manager's output
	addOutputLink(source, final, final_ch, out_ch);
}

void BlockManager::setTargetFPS(uint fps) { m_fps = fps; }

void BlockManager::setSingleShot(bool single) { m_singleShot = single; }

void BlockManager::setBufferSize(size_t size)
{
	for(auto it = m_blockLinks.keyValueBegin(); it != m_blockLinks.keyValueEnd(); ++it) {
		it->first->setBufferSize(size);
	}
	m_globalBufferSize = size;
}

void BlockManager::setPlotSize(size_t size)
{
	for(auto it = m_blockLinks.keyValueBegin(); it != m_blockLinks.keyValueEnd(); ++it) {
		it->first->setPlotSize(size);
	}
	m_globalPlotSize = size;
}

QString BlockManager::name() { return m_name; }

bool BlockManager::singleShot() {
	return m_singleShot;
}

void BlockManager::connectBlockToFilter(BasicBlock *block, uint block_ch, uint filter_ch, FilterBlock *filter)
{
	connect(
		block, &SourceBlock::newData, filter,
		[=](ChannelDataVector data, uint ch) {
			if(block_ch == ch) {
				filter->onNewData(data, filter_ch);
			}
		},
		Qt::QueuedConnection);

	filter->addConnectedChannel(filter_ch);
}

void BlockManager::disconnectBlockToFilter(BasicBlock *block, uint block_ch, uint filter_ch, FilterBlock *filter)
{
	QObject::disconnect(block, &SourceBlock::newData, filter, nullptr);

	filter->removeConnectedChannel(filter_ch);
}

void BlockManager::refilAqcCounter()
{
	// if(!m_singleShot) {
	// 	m_aqcCounter = -1;
	// 	return;
	// }

	m_aqcCounter = (m_globalPlotSize + m_globalBufferSize - 1) / m_globalBufferSize;
}

bool BlockManager::start()
{
	// a single shot is requested while running
	// if(m_running && m_singleShot) {
	// 	Q_EMIT stopped();
	// 	m_running = false;
	// }

	m_running = true;

	// if(m_singleShot) {
	// } else {
		// m_aqcCounter = 1;
	// }
	// m_runningSingleShot = m_singleShot;

	if(m_running) {
		m_fpsTimer->restart();
		m_fpsTimeElapsed = m_fpsTimer->elapsed();
	}

	refilAqcCounter();
	int tmpCounter = m_aqcCounter;
	for(int i = 0; i < tmpCounter; i++)
		for(auto it = m_blockLinks.keyValueBegin(); it != m_blockLinks.keyValueEnd(); ++it) {
			onRequestData(it->first);
			// Q_EMIT it->first->requestData();
		}

	Q_EMIT started();
	return true;
}

void BlockManager::stop()
{
	m_running = false;
	Q_EMIT stopped();
}
