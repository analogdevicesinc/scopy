#include "grxysinkcomponent.h"

#include "grtimechannelcomponent.h"
#include <gr-util/grsignalpath.h>

using namespace scopy::adc;
using namespace scopy::grutil;

#if 0

GRXySinkComponent::GRXySinkComponent(QString name, GRTopBlockNode *t, QObject *parent) : QObject(parent) {
	m_node = t;
	m_top = t->src();
	m_name = name;
	m_singleShot = false;
	m_syncMode = false;
	m_xChannel = nullptr;
}

GRXySinkComponent::~GRXySinkComponent() {

}


void GRXySinkComponent::connectSignalPaths() {
	QList<GRSignalPath *> sigpaths;
	// for through grdevices - get sampleRate;

	std::unique_lock lock(refillMutex);
	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRXYSINKCOMPONENT) << "Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(m_name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRXYSINKCOMPONENT) << "Appended " << sigpath->name();
	}

	time_sink = time_sink_f::make(m_currentSamplingInfo.plotSize, m_currentSamplingInfo.sampleRate,
				      m_name.toStdString(), sigpaths.count());
	time_sink->setSingleShot(m_singleShot);

	int index = 0;
	time_channel_map.clear();

	for(GRXyChannelComponent *gr : qAsConst(m_channels)) {
		GRSignalPath *sigPath = gr->signalPath();
		if(sigPath->enabled()) {
			// connect end of signal path to time_sink input
			m_top->connect(sigPath->getGrEndPoint(), 0, time_sink, index);
			// map signal path to time_sink input
			time_channel_map.insert(sigPath->name(), index);
			index++;
		}
	}
}

void GRXySinkComponent::tearDownSignalPaths()
{
	setCurveData(false);
	qInfo()<<"TEARING DOWN";
}

void GRXySinkComponent::setXChannelData(const float *data)
{
	m_xData = data;
}

void GRXySinkComponent::setXChannel(GRXyChannelComponent *c)
{
	if(c != nullptr) {
		if(m_xChannel) {
			if(!m_xChannel->enabled() ) {
				m_xChannel->disableChannel();
			}
		}

		m_xChannel = c;
		m_xChannel->enableChannel();
	}
}

size_t GRXySinkComponent::updateData()
{
	std::unique_lock lock(refillMutex);
	if(!time_sink)
		return false;
	uint64_t new_samples = time_sink->updateData();
	return new_samples;
}

bool GRXySinkComponent::finished() {
	return time_sink->finishedAcquisition();
}

void GRXySinkComponent::setCurveData(bool raw)
{
	int index = 0;
	if(m_xChannel) {
		int idx = m_channels.indexOf(m_xChannel);
		if(idx != -1) {
			setXChannelData(time_sink->data()[idx].data());
		} else {
			qWarning(CAT_GRXYSINKCOMPONENT) << "Cannot find m_xChannel in m_channels";
		}
	}

	index = 0;
	for(GRXyChannelComponent *gr : qAsConst(m_channels)) {
		if(gr->signalPath()->enabled()) {
			QwtPlotCurve *curve = gr->plotCh()->curve();
			const float *xdata = m_xData;
			const float *ydata = time_sink->data()[index].data();
			const size_t size = time_sink->data()[index].size();

			if(raw) {
				curve->setRawSamples(xdata, ydata, size);
			} else {
				curve->setSamples(xdata, ydata, size);
			}
			gr->onNewData(xdata, ydata, size);
			index++;
		}
	}
}


void GRXySinkComponent::setBufferSize(uint32_t size)
{
	m_currentSamplingInfo.bufferSize = size;
	Q_EMIT requestRebuild();
}

void GRXySinkComponent::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRXySinkComponent::onStart() {

	connect(this, &GRXySinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
	connect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));

	if(!m_syncMode) {
		m_top->build();
		m_top->start();
	}
}

void GRXySinkComponent::onStop() {
	if(!m_syncMode) {
		m_top->stop();
		m_top->teardown();
	}

	disconnect(this, &GRXySinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild);
	disconnect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
}

void GRXySinkComponent::onInit() {
	m_currentSamplingInfo.sampleRate = 1;
	m_currentSamplingInfo.bufferSize = 32;
	m_currentSamplingInfo.plotSize = 32;
}

void GRXySinkComponent::onDeinit() {
	qDebug(CAT_GRXYSINKCOMPONENT) << "Deinit";
	onStop();
}


void GRXySinkComponent::addChannel(ChannelComponent *c) {
	GRXyChannelComponent* chan = dynamic_cast<GRXyChannelComponent*>(c);
	if(!chan)
		return;

	if(m_xChannel == nullptr) {
		m_xChannel = chan;
	}
	m_channels.append(chan);
}

void GRXySinkComponent::removeChannel(ChannelComponent *c) {
	GRXyChannelComponent* chan = dynamic_cast<GRXyChannelComponent*>(c);
	if(chan) {
		m_channels.removeAll(chan);
	}
}

bool GRXySinkComponent::enabled() const
{
	return m_enabled;
}
#endif
