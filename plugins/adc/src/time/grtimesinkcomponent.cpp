#include "grtimesinkcomponent.h"
#include <gr-util/grsignalpath.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_GRTIMESINKCOMPONENT, "GRTimeSinkComponent")
using namespace scopy::adc;
using namespace scopy::grutil;

GRTimeSinkComponent::GRTimeSinkComponent(QString name, GRTopBlockNode *t, QObject *parent)
	: QObject(parent)
	, ToolComponent()
{
	m_enabled = true;
	m_node = t;
	m_top = t->src();
	m_name = name;
	m_rollingMode = false;
	m_singleShot = false;
	m_syncMode = false;
	m_priority = 10;
}

GRTimeSinkComponent::~GRTimeSinkComponent() {}

void GRTimeSinkComponent::connectSignalPaths()
{
	QList<GRSignalPath *> sigpaths;

	// for through grdevices - get sampleRate;
	std::unique_lock lock(refillMutex);
	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRTIMESINKCOMPONENT) << "Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(m_name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRTIMESINKCOMPONENT) << "Appended " << sigpath->name();
	}

	time_sink = time_sink_f::make(m_currentSamplingInfo.plotSize, m_currentSamplingInfo.sampleRate,
				      m_name.toStdString(), sigpaths.count());
	time_sink->setRollingMode(m_rollingMode);
	time_sink->setSingleShot(m_singleShot);

	int index = 0;
	time_channel_map.clear();

	for(GRChannel *gr : qAsConst(m_channels)) {
		GRSignalPath *sigPath = gr->sigpath();
		if(sigPath->enabled()) {
			// connect end of signal path to time_sink input
			m_top->connect(sigPath->getGrEndPoint(), 0, time_sink, index);
			// map signal path to time_sink input
			time_channel_map.insert(sigPath->name(), index);

			index++;
		}
	}
}

void GRTimeSinkComponent::tearDownSignalPaths()
{
	setData(true);
	qInfo() << "TEARING DOWN";
}

size_t GRTimeSinkComponent::updateData()
{
	std::unique_lock lock(refillMutex);
	if(!time_sink)
		return false;
	uint64_t new_samples = time_sink->updateData();
	return new_samples;
}

bool GRTimeSinkComponent::finished() { return (time_sink) ? time_sink->finishedAcquisition() : false; }

void GRTimeSinkComponent::setData(bool copy)
{
	int index = 0;

	for(GRChannel *gr : qAsConst(m_channels)) {
		int index = time_channel_map.value(gr->sigpath()->name(), -1);
		if(index == -1)
			continue;

		const float *xdata = time_sink->time().data();
		const float *ydata = time_sink->data()[index].data();
		const size_t size = time_sink->data()[index].size();

		gr->onNewData(xdata, ydata, size, copy);
	}
}

void GRTimeSinkComponent::setRollingMode(bool b)
{
	m_rollingMode = b;
	if(time_sink) {
		time_sink->setRollingMode(b);
		Q_EMIT requestRebuild();
		// updateXAxis();
	}
}

void GRTimeSinkComponent::setSampleRate(double val)
{
	m_currentSamplingInfo.sampleRate = val;
	Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setBufferSize(uint32_t size)
{
	m_currentSamplingInfo.bufferSize = size;
	Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setPlotSize(uint32_t size)
{
	m_currentSamplingInfo.plotSize = size;
	Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRTimeSinkComponent::onStart()
{

	connect(this, &GRTimeSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
	connect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));

	if(!m_syncMode) {
		m_top->build();
		m_top->start();
	}
}

void GRTimeSinkComponent::onStop()
{
	if(!m_syncMode) {
		m_top->stop();
		m_top->teardown();
	}

	disconnect(this, &GRTimeSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild);
	disconnect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
}

void GRTimeSinkComponent::onInit()
{
	m_currentSamplingInfo.sampleRate = 1;
	m_currentSamplingInfo.bufferSize = 32;
	m_currentSamplingInfo.plotSize = 32;
}

void GRTimeSinkComponent::onDeinit()
{

	qDebug(CAT_GRTIMESINKCOMPONENT) << "Deinit";
	onStop();
}

void GRTimeSinkComponent::addChannel(GRChannel *ch) { m_channels.append(ch); }

void GRTimeSinkComponent::removeChannel(GRChannel *ch) { m_channels.removeAll(ch); }
