#include "grtimesinkcomponent.h"
#include <gr-util/grsignalpath.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_GRTIMESINKCOMPONENT, "GRTimeSinkComponent")
using namespace scopy::adc;
using namespace scopy::grutil;

GRTimeSinkComponent::GRTimeSinkComponent(QString name, GRTopBlockNode *t, QObject *parent)
	: QObject(parent)
{
	m_node = t;
	m_sync = m_node->sync();
	m_top = t->src();
	m_name = name;
	m_rollingMode = false;
	m_singleShot = false;
	m_syncMode = false;
	m_armed = false;
	init();
	m_sync->addInstrument(this);


}

GRTimeSinkComponent::~GRTimeSinkComponent() {
	m_sync->removeInstrument(this);
}

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
		if(m_armed)
			Q_EMIT requestRebuild();
		// updateXAxis();
	}
}

void GRTimeSinkComponent::setSampleRate(double val)
{
	m_currentSamplingInfo.sampleRate = val;
	if(m_armed)
		Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setBufferSize(uint32_t size)
{
	m_currentSamplingInfo.bufferSize = size;
	if(m_armed)
		Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setPlotSize(uint32_t size)
{
	m_currentSamplingInfo.plotSize = size;
	if(m_armed)
		Q_EMIT requestRebuild();
}

void GRTimeSinkComponent::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRTimeSinkComponent::onArm()
{
	connect(this, &GRTimeSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
	connect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	connect(m_top, SIGNAL(started()), this, SIGNAL(ready()));
	connect(m_top, SIGNAL(aboutToStop()), this, SIGNAL(finish()));
	Q_EMIT arm();
	m_armed = true;
}

void GRTimeSinkComponent::onDisarm()
{
	m_armed = false;
	Q_EMIT disarm();
	disconnect(this, &GRTimeSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild);
	disconnect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	disconnect(m_top, SIGNAL(started()), this, SIGNAL(ready()));
	disconnect(m_top, SIGNAL(aboutToStop()), this, SIGNAL(finish()));
}

void GRTimeSinkComponent::init()
{
	m_currentSamplingInfo.sampleRate = 1;
	m_currentSamplingInfo.bufferSize = 32;
	m_currentSamplingInfo.plotSize = 32;
}

void GRTimeSinkComponent::deinit()
{
	qDebug(CAT_GRTIMESINKCOMPONENT) << "Deinit";
}

void GRTimeSinkComponent::start()
{
	m_sync->setBufferSize(this, m_currentSamplingInfo.bufferSize);
	m_sync->setSingleShot(this, m_singleShot);
	m_sync->arm(this);
	m_top->build();
	m_top->start();

}

void GRTimeSinkComponent::stop()
{
	m_top->stop();
	m_top->teardown();
	m_sync->disarm(this);

}

bool GRTimeSinkComponent::syncMode() {
	return m_syncMode;
}

void GRTimeSinkComponent::setSyncMode(bool b)
{
	m_syncMode = b;
}

void GRTimeSinkComponent::setSyncController(SyncController *s)
{
	m_sync = s;
}

void GRTimeSinkComponent::addChannel(GRChannel *ch) { m_channels.append(ch); }

void GRTimeSinkComponent::removeChannel(GRChannel *ch) { m_channels.removeAll(ch); }

void GRTimeSinkComponent::setSyncSingleShot(bool b)
{
	Q_EMIT requestSingleShot(b);
}

void GRTimeSinkComponent::setSyncBufferSize(uint32_t val)
{
	Q_EMIT requestBufferSize(val);
}

const QString &GRTimeSinkComponent::name() const
{
	return m_name;
}
