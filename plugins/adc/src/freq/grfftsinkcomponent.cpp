#include "grfftsinkcomponent.h"
#include <gr-util/grsignalpath.h>
#include <QLoggingCategory>
#include "iioutil/iiopingtask.h"
#include "pluginbase/preferences.h"

Q_LOGGING_CATEGORY(CAT_GRFFTSINKCOMPONENT, "GRFFTSinkComponent")
using namespace scopy::adc;
using namespace scopy::grutil;

GRFFTSinkComponent::GRFFTSinkComponent(QString name, GRTopBlockNode *t, QObject *parent)
	: QObject(parent)
{
	m_node = t;
	m_sync = m_node->sync();
	m_top = t->src();
	m_name = name;
	m_singleShot = false;
	m_syncMode = false;
	m_armed = false;
	init();
	m_sync->addInstrument(this);
}

GRFFTSinkComponent::~GRFFTSinkComponent() { m_sync->removeInstrument(this); }

void GRFFTSinkComponent::connectSignalPaths()
{
	QList<GRSignalPath *> sigpaths;

	// for through grdevices - get sampleRate;
	std::unique_lock lock(refillMutex);
	for(auto &sigpath : m_top->signalPaths()) {
		qDebug(CAT_GRFFTSINKCOMPONENT) << "Trying " << sigpath->name();
		if(!sigpath->enabled())
			continue;
		if(!sigpath->name().startsWith(m_name))
			continue;
		sigpaths.append(sigpath);
		qDebug(CAT_GRFFTSINKCOMPONENT) << "Appended " << sigpath->name();
	}

	time_sink = time_sink_f::make(m_samplingInfo.bufferSize, m_samplingInfo.bufferSize, m_samplingInfo.sampleRate,
				      m_name.toStdString(), sigpaths.count());
	time_sink->setRollingMode(false);
	time_sink->setSingleShot(m_singleShot);
	time_sink->setFftComplex(m_samplingInfo.complexMode);
	time_sink->setFreqOffset(m_samplingInfo.freqOffset);

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

void GRFFTSinkComponent::tearDownSignalPaths()
{
	setData(true);
	qInfo() << "TEARING DOWN";
}

size_t GRFFTSinkComponent::updateData()
{
	std::unique_lock lock(refillMutex);
	if(!time_sink)
		return false;
	uint64_t new_samples = time_sink->updateData();
	return new_samples;
}

bool GRFFTSinkComponent::finished() { return (time_sink) ? time_sink->finishedAcquisition() : false; }

void GRFFTSinkComponent::setData(bool copy)
{
	int index = 0;

	for(GRChannel *gr : qAsConst(m_channels)) {
		int index = time_channel_map.value(gr->sigpath()->name(), -1);
		if(index == -1)
			continue;

		const float *xdata = time_sink->freq().data();
		const float *ydata = time_sink->data()[index].data();
		const size_t size = time_sink->data()[index].size();

		gr->onNewData(xdata, ydata, size, copy);
	}
}

SamplingInfo GRFFTSinkComponent::samplingInfo() { return m_samplingInfo; }

void GRFFTSinkComponent::setSamplingInfo(SamplingInfo p)
{
	m_samplingInfo = p;
	m_top->setVLen(m_samplingInfo.bufferSize);
	if(m_armed)
		Q_EMIT requestRebuild();
}

void GRFFTSinkComponent::setSingleShot(bool b)
{
	m_singleShot = b;
	if(time_sink) {
		time_sink->setSingleShot(m_singleShot);
	}
}

void GRFFTSinkComponent::onArm()
{
	connect(this, &GRFFTSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild, Qt::QueuedConnection);
	connect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	connect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	connect(m_top, SIGNAL(started()), this, SIGNAL(ready()));
	connect(m_top, SIGNAL(aboutToStop()), this, SIGNAL(finish()));
	Q_EMIT arm();
	m_armed = true;
}

void GRFFTSinkComponent::onDisarm()
{
	m_armed = false;
	Q_EMIT disarm();
	disconnect(this, &GRFFTSinkComponent::requestRebuild, m_top, &GRTopBlock::rebuild);
	disconnect(m_top, SIGNAL(builtSignalPaths()), this, SLOT(connectSignalPaths()));
	disconnect(m_top, SIGNAL(teardownSignalPaths()), this, SLOT(tearDownSignalPaths()));
	disconnect(m_top, SIGNAL(started()), this, SIGNAL(ready()));
	disconnect(m_top, SIGNAL(aboutToStop()), this, SIGNAL(finish()));
}

void GRFFTSinkComponent::init()
{
	m_samplingInfo.sampleRate = 1;
	m_samplingInfo.bufferSize = 32;
	m_samplingInfo.plotSize = 32;
}

void GRFFTSinkComponent::deinit() { qDebug(CAT_GRFFTSINKCOMPONENT) << "Deinit"; }

bool GRFFTSinkComponent::start()
{
	bool ok;
	double timeout = Preferences::get("adc_acquisition_timeout").toDouble(&ok);
	if(!ok)
		timeout = 1000;

	iio_context_set_timeout(m_node->ctx(), 1000);
	if(!IIOPingTask::pingCtx(m_node->ctx()))
		return false;
	iio_context_set_timeout(m_node->ctx(), timeout);
	m_sync->setBufferSize(this, m_samplingInfo.bufferSize);
	m_sync->setSingleShot(this, m_singleShot);
	m_top->setVLen(m_samplingInfo.bufferSize);
	m_sync->arm(this);
	m_top->build();
	m_top->start();
	return true;
}

void GRFFTSinkComponent::stop()
{
	m_top->stop();
	m_top->teardown();
	m_sync->disarm(this);
}

bool GRFFTSinkComponent::syncMode() { return m_syncMode; }

void GRFFTSinkComponent::setSyncMode(bool b) { m_syncMode = b; }

void GRFFTSinkComponent::setSyncController(SyncController *s) { m_sync = s; }

void GRFFTSinkComponent::addChannel(GRChannel *ch) { m_channels.append(ch); }

void GRFFTSinkComponent::removeChannel(GRChannel *ch) { m_channels.removeAll(ch); }

void GRFFTSinkComponent::setSyncSingleShot(bool b) { Q_EMIT requestSingleShot(b); }

void GRFFTSinkComponent::setSyncBufferSize(uint32_t val) { Q_EMIT requestBufferSize(val); }

const QString &GRFFTSinkComponent::name() const { return m_name; }
