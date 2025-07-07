#include <include/data-sink/timechannelsigpath.h>

using namespace scopy::datasink;
using namespace scopy;

OffsetFilter::OffsetFilter(QString name)
	: FilterBlock(false, name)
	, m_offset(0.0)
	, m_scale(1.0)
{}

void OffsetFilter::setOffset(double offset) { m_offset = offset; }

void OffsetFilter::setScale(double scale) { m_scale = scale; }

double OffsetFilter::getOffset() const { return m_offset; }

double OffsetFilter::getScale() const { return m_scale; }

ChannelDataVector *OffsetFilter::createData()
{
	if(!m_data || m_data->isEmpty()) {
		return new ChannelDataVector();
	}

	// Get the first available channel data (assuming single channel processing)
	// You might need to modify this based on your specific channel handling
	auto it = m_data->begin();
	if(it == m_data->end()) {
		return new ChannelDataVector();
	}

	ChannelDataVector *inputData = it.value();
	if(!inputData || inputData->data.empty()) {
		return new ChannelDataVector();
	}
	// return new ChannelDataVector(inputData->data.size());

	// Create output data with same size
	ChannelDataVector *outputData = new ChannelDataVector(inputData->data.size());

	// return outputData;

	// Apply scale and offset: output = (input * scale) + offset
	// outputData->data.resize(inputData->data.size());
	for(int i = 0; i < inputData->data.size(); i++) {
		outputData->data.push_back((inputData->data[i] * m_scale) + m_offset);
	}
	// std::transform(inputData->data.begin(), inputData->data.end(),
	// 	       outputData->data.begin(),
	// 	       [this](float value) {
	// 		       return (value * m_scale) + m_offset;
	// 	       });

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

	// Create offset filter
	m_offsetFilter = new OffsetFilter("offset_" + name);
	m_signalPath->addFilter(m_offsetFilter);

	// Connect to manager's newData signal
	connect(m_manager, &BlockManager::newData, this, &TimeChannelSigpath::onManagerNewData);

	if(manager) {
		if(!m_filterConnected) {
			BlockManager::connectBlockToFilter(m_sourceBlock, m_sourceChannel, 0, m_offsetFilter);
			m_filterConnected = true;
		}
		m_manager->addLink(m_sourceBlock, m_sourceChannel, m_offsetFilter, 0, m_outputChannel, true);
	}
}

TimeChannelSigpath::~TimeChannelSigpath()
{
	if(m_enabled) {
		disable();
	}

	// Clean up the offset filter
	if(m_offsetFilter) {
		delete m_offsetFilter;
		m_offsetFilter = nullptr;
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

void TimeChannelSigpath::onManagerNewData(ChannelDataVector *data, uint ch)
{
	// Forward data to channel component if this is our output channel
	if(ch == m_outputChannel && m_ch) {
		// forwardDataToChannel(data);
	}
}

void TimeChannelSigpath::forwardDataToChannel(ChannelDataVector *data)
{
	// Forward the processed data to the channel component
	// This depends on your ChannelComponent interface
	// Since the original code called m_ch->chData()->onNewData(xData, yData, size, copy)
	// and now we have ChannelDataVector, you'll need to adapt this based on your
	// ChannelComponent's expected interface

	if(m_ch && data && !data->data.empty()) {
		// Example implementation - you'll need to adapt this to your actual interface:
		// If your ChannelComponent expects separate X and Y data:
		//
		// Option 1: If data contains interleaved X,Y pairs
		// size_t numSamples = data->data.size() / 2;
		// const float* xData = data->data.data();
		// const float* yData = data->data.data() + numSamples;
		// m_ch->chData()->onNewData(xData, yData, numSamples, false);
		//
		// Option 2: If data is Y-only and X is generated (time domain)
		// You might generate X data or use existing time base
		// std::vector<float> xData; // generate time values
		// m_ch->chData()->onNewData(xData.data(), data->data.data(), data->data.size(), false);
		//
		// Option 3: If ChannelComponent now accepts ChannelDataVector directly
		// m_ch->chData()->onNewData(data);

		// Placeholder - implement based on your actual ChannelComponent interface
		// For now, assuming you need to implement this based on your specific needs
		float *xData = new float[data->data.size()];
		for(int i = 0; i < data->data.size(); ++i) {
			xData[i] = static_cast<float>(i);
		}
		m_ch->chData()->onNewData(xData, data->data.data(), data->data.size(), true);
		// delete data;
		delete[] xData;
	}
}
