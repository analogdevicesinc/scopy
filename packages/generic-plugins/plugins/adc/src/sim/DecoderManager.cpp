#include "DecoderManager.h"

#include "DecoderOverlay.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/ExternalDecoderProcessor.h>
#include <core/decoder/DecoderLogger.h>
#include <core/decoder/IDecoderBackendFactory.h>

#include <gui/plotaxis.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <QPen>

namespace scopy {
namespace adc {

static constexpr const char *kMgrId = "decoder-manager";

DecoderManager::DecoderManager(scopy::acq::AcquisitionEngine *engine,
                               scopy::acq::DataStore *store,
                               scopy::decoder::IDecoderBackendFactory *backendFactory,
                               QObject *parent)
	: QObject(parent)
	, m_engine(engine)
	, m_store(store)
	, m_backendFactory(backendFactory)
{}

DecoderManager::~DecoderManager()
{
	// Detach every processor from the engine before destruction so the
	// engine doesn't call process() on dangling pointers.
	for(const DecoderInstance &d : m_decoders) {
		if(m_engine && d.proc) m_engine->removeProcessor(d.proc);
		if(d.proc) d.proc->deleteLater();
	}
	m_decoders.clear();
}

void DecoderManager::setPlot(scopy::PlotWidget *plot)    { m_plot = plot; }
void DecoderManager::setOverlay(DecoderOverlay *overlay) { m_overlay = overlay; }

bool DecoderManager::isEngineRunning() const
{
	return m_engine && m_engine->isRunning();
}

DecoderInstance *DecoderManager::find(const QString &uid)
{
	for(DecoderInstance &d : m_decoders)
		if(d.uid == uid) return &d;
	return nullptr;
}

scopy::PlotAxis *DecoderManager::allocateBand()
{
	if(!m_plot) return nullptr;
	constexpr double kBandHeight = 1.0;
	const double top    = m_nextBandTop;
	const double bottom = top - kBandHeight;
	m_nextBandTop       = bottom;

	QPen pen(scopy::Style::getColor(json::theme::content_silent));
	auto *axis = new scopy::PlotAxis(m_plot->yAxis()->position(),
	                                 m_plot, pen, this);
	axis->setInterval(bottom, top);
	Q_EMIT bandAllocated(axis);
	return axis;
}

QString DecoderManager::addDecoder(const QString &decoderId)
{
	if(!m_engine) {
		if(m_logger) m_logger->critical(kMgrId, QStringLiteral("addDecoder: no engine"));
		return {};
	}
	if(!m_overlay || !m_plot) {
		if(m_logger)
			m_logger->warning(kMgrId,
				QStringLiteral("addDecoder: overlay/plot not set (call setPlot/setOverlay first)"));
		return {};
	}
	if(!m_backendFactory) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: no backend factory injected"));
		return {};
	}
	if(decoderId.isEmpty()) return {};

	const QString uid = QString("%1#%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: backend factory returned null"));
		return {};
	}
	auto *proc   = new scopy::acq::ExternalDecoderProcessor(
		uid, std::move(backend), m_engine);

	// Minimal default config; user must Apply an updated one for anything
	// meaningful to happen (channel bindings are empty until then).
	scopy::decoder::DecoderConfig cfg;
	cfg.decoderId   = decoderId.toStdString();
	cfg.sampleRate  = 1.0e6;
	cfg.numChannels = 0;
	proc->setConfig(cfg);
	proc->setWindowSize(m_windowSize);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder", uid, "annotations");
	proc->setOutputKey(outKey);

	m_engine->addProcessor(proc);

	scopy::PlotAxis *axis = allocateBand();
	m_overlay->registerDecoder(proc, axis);

	DecoderInstance d;
	d.uid       = uid;
	d.decoderId = decoderId;
	d.cfg       = cfg;
	d.proc      = proc;
	d.outKey    = outKey;
	d.axis      = axis;
	m_decoders.append(d);

	Q_EMIT decoderAdded(uid);
	if(m_logger) m_logger->info(kMgrId, QStringLiteral("added decoder ") + uid);
	return uid;
}

void DecoderManager::removeDecoder(const QString &uid)
{
	for(int i = 0; i < m_decoders.size(); ++i) {
		if(m_decoders[i].uid != uid) continue;
		DecoderInstance d = m_decoders.takeAt(i);
		if(m_engine && d.proc) m_engine->removeProcessor(d.proc);
		if(m_overlay) m_overlay->unregisterDecoder(d.outKey);
		if(m_store)   m_store->remove(d.outKey);
		if(d.proc) d.proc->deleteLater();
		Q_EMIT decoderRemoved(uid);
		if(m_logger) m_logger->info(kMgrId, QStringLiteral("removed decoder ") + uid);
		return;
	}
}

void DecoderManager::applyConfig(const QString &uid,
                                 const scopy::decoder::DecoderConfig &cfg,
                                 const QList<scopy::acq::DataKey> &orderedRawKeys)
{
	DecoderInstance *d = find(uid);
	if(!d || !d->proc) {
		if(m_logger)
			m_logger->warning(kMgrId, QStringLiteral("applyConfig: unknown uid ") + uid);
		return;
	}
	if(isEngineRunning()) {
		if(m_logger)
			m_logger->warning(kMgrId,
				QStringLiteral("applyConfig: refusing mid-run mutation for ") + uid);
		return;
	}

	d->cfg            = cfg;
	d->orderedRawKeys = orderedRawKeys;
	d->proc->setConfig(cfg);
	d->proc->setOrderedRawKeys(orderedRawKeys);
	d->proc->setWatchedKeys(orderedRawKeys);

	if(m_store && m_windowSize > 0) {
		// Ensure the decoder can stitch a full window regardless of
		// what the display currently reads.
		const std::size_t depth = scopy::acq::DataStore::requiredHistoryDepth(
			static_cast<std::size_t>(m_windowSize),
			m_engine ? m_engine->bufferSize() : 1);
		for(const scopy::acq::DataKey &k : orderedRawKeys)
			m_store->ensureHistoryDepth(k, depth);
	}

	if(m_logger)
		m_logger->info(kMgrId,
			QStringLiteral("applied config to %1 channels=%2 sampleRate=%3")
				.arg(uid).arg(orderedRawKeys.size()).arg(cfg.sampleRate));
	Q_EMIT configApplied(uid);
}

void DecoderManager::setDecoderWindowSize(int n)
{
	m_windowSize = n;
	for(const DecoderInstance &d : m_decoders)
		if(d.proc) d.proc->setWindowSize(n);
}

} // namespace adc
} // namespace scopy
