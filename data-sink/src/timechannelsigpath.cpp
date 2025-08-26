#include "include/data-sink/customSourceBlocks.h"
#include <include/data-sink/timechannelsigpath.h>
#include <iostream>

using namespace scopy::datasink;
using namespace scopy;

RollingBufferFilter::RollingBufferFilter(SourceBlock* source, QString name)
	: FilterBlock(false, name)
	, m_plotSize(0)
	, m_rollingMode(false)
	, m_currentPosition(0)
	, m_bufferFull(false)
	, m_source(source)
{
	connect(source, &IIOSourceBlock::plotSizeChanged, this, &RollingBufferFilter::setPlotSize);
	connect(source, &IIOSourceBlock::resetPlotBuffer, this, &RollingBufferFilter::resetPlotBuffer);
}

void RollingBufferFilter::setPlotSize(size_t plotSize)
{
	if(plotSize == 0) {
		return;
	}
	m_plotSize = plotSize;
	resetPlotBuffer();
}

void RollingBufferFilter::setRollingMode(bool rolling)
{
	if(m_rollingMode != rolling) {
		m_rollingMode = rolling;
		resetPlotBuffer();
	}
}

size_t RollingBufferFilter::getPlotSize() const { return m_plotSize; }

bool RollingBufferFilter::isRollingMode() const { return m_rollingMode; }

void RollingBufferFilter::resetPlotBuffer()
{
	m_plotBuffer.data.clear();
	if(m_plotSize > 0) {
		m_plotBuffer.data.reserve(m_plotSize);
	}
	m_currentPosition = 0;
	m_bufferFull = false;
}

ChannelDataVector RollingBufferFilter::createData()
{
	if(m_plotSize == 0) {
		return ChannelDataVector();
	}

	// Check if channel 0 exists in m_data
	if(!m_data.contains(0)) {
		return ChannelDataVector(m_plotBuffer.data);
	}

	std::vector<float> &newData = m_data[0].data;

	if(newData.empty()) {
		return ChannelDataVector(m_plotBuffer.data);
	}

	// Check if bufferSize == plotSize for direct optimization
	if(newData.size() >= m_plotSize) {
		newData.resize(m_plotSize);
		// Direct reference - no copying needed, no reversal in rolling mode
		return ChannelDataVector(newData);
	}

	// Original logic for when bufferSize != plotSize
	if(!m_rollingMode) {
		if(m_plotBuffer.data.size() >= m_plotSize) {
			resetPlotBuffer();
		}

		// Non-rolling mode: append until full, then clear and start again
		for(const float &sample : newData) {
			m_plotBuffer.data.push_back(sample);
		}
		if(m_plotBuffer.data.size() > m_plotSize) {
			m_plotBuffer.data.resize(m_plotSize);
		}

		std::cout << "--- non rolling: " << m_plotBuffer.data.size() << std::endl;
		return ChannelDataVector(m_plotBuffer.data);
	} else {
		// Rolling mode with circular buffer (most efficient)
		// Ensure buffer is allocated to full size
		if(m_plotBuffer.data.size() < m_plotSize) {
			m_plotBuffer.data.resize(m_plotSize, 0.0f);
		}

		for(const float &sample : newData) {
			if(m_currentPosition >= m_plotSize) {
				break;
			}

			m_plotBuffer.data[m_currentPosition] = sample;
			m_currentPosition = (m_currentPosition + 1) % m_plotSize;
			if(m_currentPosition == 0) {
				m_bufferFull = true;
			}
		}

		// Return data in correct order (oldest to newest)
		if(m_bufferFull) {
			std::vector<float> orderedData;
			orderedData.reserve(m_plotSize);

			// Copy from current position to end (oldest data)
			for(size_t i = m_currentPosition; i < m_plotSize; ++i) {
				orderedData.push_back(m_plotBuffer.data[i]);
			}
			// Copy from start to current position (newest data)
			for(size_t i = 0; i < m_currentPosition; ++i) {
				orderedData.push_back(m_plotBuffer.data[i]);
			}

			std::cout << "--- full rolling sent: " << orderedData.size() << std::endl;
			return ChannelDataVector(std::move(orderedData));
		} else {
			// Buffer not full yet, return what we have in order
			std::vector<float> orderedData(m_plotBuffer.data.begin(),
						       m_plotBuffer.data.begin() + m_currentPosition);
			std::cout << "--- rolling sent: " << orderedData.size() << std::endl;
			return ChannelDataVector(std::move(orderedData));
		}
	}
}

OffsetFilter::OffsetFilter(QString name)
	: FilterBlock(false, name)
	, m_offset(0.0)
	, m_scale(1.0)
{}

void OffsetFilter::setOffset(double offset) { m_offset = offset; }

void OffsetFilter::setScale(double scale) { m_scale = scale; }

double OffsetFilter::getOffset() const { return m_offset; }

double OffsetFilter::getScale() const { return m_scale; }

ChannelDataVector OffsetFilter::createData()
{
	if(m_data.isEmpty()) {
		return ChannelDataVector();
	}

	// Get the first available channel data (assuming single channel processing)
	// You might need to modify this based on your specific channel handling
	auto it = m_data.begin();
	if(it == m_data.end()) {
		return ChannelDataVector();
	}

	ChannelDataVector inputData = it.value();
	if(inputData.data.empty()) {
		return ChannelDataVector();
	}
	// return new ChannelDataVector(inputData->data.size());

	// Create output data with same size
	ChannelDataVector outputData = ChannelDataVector(inputData.data.size());

	// return outputData;

	// Apply scale and offset: output = (input * scale) + offset
	// outputData->data.resize(inputData->data.size());
	for(int i = 0; i < inputData.data.size(); i++) {
		outputData.data.push_back((inputData.data[i] * m_scale) + m_offset);
	}
	// std::transform(inputData.data.begin(), inputData.data.end(),
	// 	       outputData.data.begin(),
	// 	       [this](float value) {
	// 		       return (value * m_scale) + m_offset;
	// 	       });

	std::cout << "sent offset: " << outputData.data.size() << std::endl;
	return outputData;
}

// ProxyBlock implementation
ProxyBlock::ProxyBlock(QObject *parent)
	: QObject(parent)
	, m_enabled(false)
{}

ProxyBlock::~ProxyBlock() {}

void ProxyBlock::setEnabled(bool v)
{
	if(m_enabled != v) {
		m_enabled = v;
		emit enabledChanged(m_enabled);
	}
}

bool ProxyBlock::enabled() const { return m_enabled; }

// SignalPath implementation
SignalPath::SignalPath(QString name, QObject *parent)
	: ProxyBlock(parent)
	, m_name(name)
	, m_manager(nullptr)
{}

void SignalPath::setManager(BlockManager *manager) { m_manager = manager; }

BlockManager *SignalPath::manager() const { return m_manager; }

QString SignalPath::name() const { return m_name; }

void SignalPath::addFilter(FilterBlock *filter)
{
	if(filter && !m_filters.contains(filter)) {
		m_filters.append(filter);
	}
}

void SignalPath::removeFilter(FilterBlock *filter) { m_filters.removeAll(filter); }

QList<FilterBlock *> SignalPath::filters() const { return m_filters; }

// TimeChannelSigpath implementation
TimeChannelSigpath::TimeChannelSigpath(QString name, ChannelComponent *ch, SourceBlock *sourceBlock, uint sourceChannel,
				       uint outputChannel, BlockManager *manager, QObject *parent)
	: QObject(parent)
	, m_name(name)
	, m_ch(ch)
	, m_sourceBlock(sourceBlock)
	, m_sourceChannel(sourceChannel)
	, m_outputChannel(outputChannel)
	, m_manager(manager)
	, m_enabled(false)
	, m_filterConnected(false)
{
	// Create signal path
	m_signalPath = new SignalPath(name + "_sigpath", this);
	m_signalPath->setManager(manager);

	// Create filters
	m_rollingFilter = new RollingBufferFilter(m_sourceBlock, "rolling_" + name + QString::number(sourceChannel));
	m_offsetFilter = new OffsetFilter("offset_" + name);
	m_signalPath->addFilter(m_rollingFilter);
	m_signalPath->addFilter(m_offsetFilter);

	// Connect to manager's newData signal
	connect(m_manager, &BlockManager::newData, this, &TimeChannelSigpath::onManagerNewData);

	if(manager) {
		if(!m_filterConnected) {
			BlockManager::connectBlockToFilter(m_sourceBlock, m_sourceChannel, 0, m_rollingFilter);
			BlockManager::connectBlockToFilter(m_rollingFilter, 0, 0, m_offsetFilter);
			m_filterConnected = true;
		}
		m_manager->addLink(m_sourceBlock, m_sourceChannel, m_offsetFilter, 0, m_outputChannel, true);
	}
}

TimeChannelSigpath::~TimeChannelSigpath()
{
	// if(m_enabled) {
	// 	disable();
	// }

	// Clean up the offset filter
	if(m_offsetFilter) {
		delete m_offsetFilter;
		m_offsetFilter = nullptr;
	}
	if(m_rollingFilter) {
		delete m_rollingFilter;
		m_rollingFilter = nullptr;
	}
}

void TimeChannelSigpath::enable()
{
	if(!m_enabled && m_manager) {
		m_sourceBlock->enChannel(true, m_sourceChannel);
		// Connect source to offset filter using BlockManager::connectBlockToFilter

		// Add link: source -> offset filter -> output

		m_enabled = true;
		m_signalPath->setEnabled(true);
	}
}

void TimeChannelSigpath::disable()
{
	if(m_enabled && m_manager) {
		m_sourceBlock->enChannel(false, m_sourceChannel);
		// Remove the link first
		// m_manager->removeLink(m_sourceBlock, m_sourceChannel, m_outputChannel);

		// Disconnect the filter
		// if (m_filterConnected) {
		// 	BlockManager::disconnectBlockToFilter(m_sourceBlock, m_sourceChannel, 0, m_offsetFilter);
		// 	m_filterConnected = false;
		// }

		m_enabled = false;
		m_signalPath->setEnabled(false);
	}
}

bool TimeChannelSigpath::isEnabled() const { return m_enabled; }

void TimeChannelSigpath::setOffset(double offset)
{
	if(m_offsetFilter) {
		m_offsetFilter->setOffset(offset);
	}
}

void TimeChannelSigpath::setRollingMode(bool mode)
{
	if(m_rollingFilter) {
		m_rollingFilter->setRollingMode(mode);
	}
}

void TimeChannelSigpath::setScale(double scale)
{
	if(m_offsetFilter) {
		m_offsetFilter->setScale(scale);
	}
}

double TimeChannelSigpath::getOffset() const { return m_offsetFilter ? m_offsetFilter->getOffset() : 0.0; }

double TimeChannelSigpath::getScale() const { return m_offsetFilter ? m_offsetFilter->getScale() : 1.0; }

SignalPath *TimeChannelSigpath::sigpath() { return m_signalPath; }

ChannelComponent *TimeChannelSigpath::channelComponent() const { return m_ch; }

SourceBlock *TimeChannelSigpath::sourceBlock() const { return m_sourceBlock; }

uint TimeChannelSigpath::sourceChannel() const { return m_sourceChannel; }

uint TimeChannelSigpath::outputChannel() const { return m_outputChannel; }

void TimeChannelSigpath::onManagerNewData(ChannelDataVector data, uint ch)
{
	if(ch == m_outputChannel && m_ch) {
		forwardDataToChannel(data);
	}
}

void TimeChannelSigpath::forwardDataToChannel(ChannelDataVector data)
{
	if(m_ch && !data.data.empty()) {
		// time axis samp rate should be the one in GUI, not actual SR. it should be updatd on every change
		m_ch->chData()->onNewData(static_cast<IIOSourceBlock *>(m_sourceBlock)->getTimeAxis().data(),
					  data.data.data(), data.data.size(), true);
		data.clear();
	}
}
