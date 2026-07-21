#include "DecoderManager.h"

#include "DecoderOverlay.h"
#include "DigitalTrackManager.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/ExternalDecoderProcessor.h>
#include <core/decoder/DecoderLogger.h>
#include <core/decoder/IDecoderBackendFactory.h>

#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <axishandle.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include <QPen>

namespace scopy {
namespace adc {

static constexpr const char *kMgrId = "decoder-manager";

// Root stage: uid; stacked stages: "<uid> → <decoderId>".
static QString stageTitle(const QString &uid,
                          const QString &decoderId,
                          int stageIndex)
{
	if(stageIndex == 0) return uid;
	return uid + QStringLiteral(" ") + QChar(0x2192)
		+ QStringLiteral(" ") + decoderId;
}

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
	// Detach processors before destruction to avoid dangling calls.
	for(const DecoderInstance &d : m_decoders) {
		if(m_engine && d.proc) m_engine->removeProcessor(d.proc);
		for(const QPointer<scopy::PlotAxisHandle> &h : d.handles) {
			if(h.isNull()) continue;
			if(!m_digitalMgr.isNull()) {
				m_digitalMgr->unregisterAnnotationBand(h);
			} else {
				if(m_plot) m_plot->removePlotAxisHandle(h);
				h->deleteLater();
			}
		}
		if(d.proc) d.proc->deleteLater();
	}
	m_decoders.clear();
}

void DecoderManager::setPlot(scopy::PlotWidget *plot)   { m_plot = plot; }
void DecoderManager::setOverlay(DecoderOverlay *overlay) { m_overlay = overlay; }
void DecoderManager::setDigitalTrackManager(DigitalTrackManager *mgr) { m_digitalMgr = mgr; }

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

double DecoderManager::nextBandPos()
{
	if(!m_plot || !m_plot->yAxis()) return 0.0;

	if(!m_bandCursorInit) {
		const double ymin = m_plot->yAxis()->min();
		const double ymax = m_plot->yAxis()->max();
		const double gap  = (ymax - ymin) * 0.05;
		m_nextBandPos     = ymin - gap;
		m_bandCursorInit  = true;
	}

	constexpr double kBandStride = 1.0;
	const double pos = m_nextBandPos;
	m_nextBandPos    = pos - kBandStride;
	return pos;
}

scopy::PlotAxisHandle *DecoderManager::attachHandle(double initialPos)
{
	if(!m_plot || !m_plot->yAxis()) return nullptr;

	auto *h = new scopy::PlotAxisHandle(m_plot, m_plot->yAxis());
	h->handle()->setBarVisibility(scopy::BarVisibility::ON_HOVER);
	h->handle()->setColor(scopy::Style::getColor(json::theme::content_silent));
	h->handle()->setHandlePos(scopy::HandlePos::SOUTH_OR_EAST);

	m_plot->addPlotAxisHandle(h);

	if(auto *ah = h->handle()) {
		if(auto *canvas = m_plot->plot()->canvas()) {
			ah->setParent(canvas);
			ah->resize(canvas->size());
		}
		ah->show();
		ah->raise();
	}

	h->setPosition(initialPos);

	connect(h, &scopy::PlotAxisHandle::scalePosChanged, this, [this](double) {
		if(m_plot) m_plot->plot()->replot();
	});

	return h;
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

	const QString uid = QString("%1-%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: backend factory returned null"));
		return {};
	}
	auto *proc   = new scopy::acq::ExternalDecoderProcessor(
		uid, std::move(backend), m_engine);

	scopy::decoder::DecoderConfig cfg;
	cfg.sampleRate  = 1.0e6;
	cfg.numChannels = 0;
	scopy::decoder::DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	cfg.stack.push_back(stage);
	proc->setConfig(cfg);
	proc->setWindowSize(m_windowSize);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder", uid + QStringLiteral("/0"),
		                                "annotations");
	proc->setOutputKeys({outKey});

	m_engine->addProcessor(proc);

	scopy::PlotAxisHandle *handle = nullptr;
	if(!m_digitalMgr.isNull()) {
		handle = m_digitalMgr->registerAnnotationBand(uid);
	} else {
		handle = attachHandle(nextBandPos());
	}
	m_overlay->registerDecoder(proc, outKey, handle,
	                           stageTitle(uid, decoderId, 0));

	DecoderInstance d;
	d.uid       = uid;
	d.stageIds  = {decoderId};
	d.cfg       = cfg;
	d.proc      = proc;
	d.outKeys   = {outKey};
	d.handles   = {QPointer<scopy::PlotAxisHandle>(handle)};
	m_decoders.append(d);

	Q_EMIT decoderAdded(uid);
	if(m_logger) m_logger->info(kMgrId, QStringLiteral("added decoder ") + uid);
	return uid;
}

QString DecoderManager::addDecoderFromAnnotations(const QString &decoderId,
                                                  const AnnotationChainSpec &spec)
{
	if(!m_engine) {
		if(m_logger) m_logger->critical(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: no engine"));
		return {};
	}
	if(!m_overlay || !m_plot) {
		if(m_logger) m_logger->warning(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: overlay/plot not set"));
		return {};
	}
	if(!m_backendFactory) {
		if(m_logger) m_logger->critical(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: no backend factory"));
		return {};
	}
	if(decoderId.isEmpty() || spec.upstreamId.isEmpty()) return {};

	DecoderInstance *src = find(spec.sourceUid);
	if(!src) {
		if(m_logger) m_logger->warning(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: unknown sourceUid %1")
				.arg(spec.sourceUid));
		return {};
	}
	if(spec.sourceStageIndex < 0 || spec.sourceStageIndex >= src->outKeys.size()) {
		if(m_logger) m_logger->warning(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: bad stage index %1 (source has %2 stages)")
				.arg(spec.sourceStageIndex).arg(src->outKeys.size()));
		return {};
	}

	scopy::decoder::DecoderConfig cfg;
	cfg.sampleRate  = src->cfg.sampleRate;
	cfg.numChannels = 0;
	scopy::decoder::DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	cfg.stack.push_back(stage);
	cfg.rootInput = scopy::decoder::RootInput::Annotations;
	cfg.annotationInput.sourceUid        = spec.sourceUid.toStdString();
	cfg.annotationInput.sourceStageIndex = spec.sourceStageIndex;

	cfg.meta["annIn.upstreamId"] = spec.upstreamId.toStdString();
	for(auto it = spec.options.constBegin(); it != spec.options.constEnd(); ++it) {
		cfg.meta[std::string("annIn.") + it.key().toStdString()] =
			it.value().toStdString();
	}

	// Pre-check backend support before allocating.
	auto probeBackend = m_backendFactory->create();
	if(!probeBackend) {
		if(m_logger) m_logger->critical(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: probe backend null"));
		return {};
	}
	if(!probeBackend->acceptsAnnotationInput(cfg)) {
		if(m_logger) m_logger->warning(kMgrId,
			QStringLiteral("addDecoderFromAnnotations: backend rejects (upstream=%1, downstream=%2)")
				.arg(spec.upstreamId, decoderId));
		return {};
	}
	probeBackend.reset();

	const QString uid = QString("%1-%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend) return {};
	auto *proc = new scopy::acq::ExternalDecoderProcessor(
		uid, std::move(backend), m_engine);
	proc->setConfig(cfg);
	proc->setWindowSize(m_windowSize);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder",
			uid + QStringLiteral("/0"), "annotations");
	proc->setOutputKeys({outKey});

	// Watch the source outKey; engine schedules on writes to watched keys.
	const scopy::acq::DataKey srcKey = src->outKeys[spec.sourceStageIndex];
	proc->setAnnotationInputKey(srcKey);
	proc->setWatchedKeys({srcKey});

	m_engine->addProcessor(proc);

	scopy::PlotAxisHandle *handle = nullptr;
	if(!m_digitalMgr.isNull()) {
		handle = m_digitalMgr->registerAnnotationBand(uid);
	} else {
		handle = attachHandle(nextBandPos());
	}
	m_overlay->registerDecoder(proc, outKey, handle,
	                           stageTitle(uid, decoderId, 0));

	DecoderInstance d;
	d.uid       = uid;
	d.stageIds  = {decoderId};
	d.cfg       = cfg;
	d.proc      = proc;
	d.outKeys   = {outKey};
	d.handles   = {QPointer<scopy::PlotAxisHandle>(handle)};
	m_decoders.append(d);

	Q_EMIT decoderAdded(uid);
	if(m_logger)
		m_logger->info(kMgrId,
			QStringLiteral("added annotation-chained decoder %1 (source=%2 stage=%3 upstream=%4)")
				.arg(uid, spec.sourceUid).arg(spec.sourceStageIndex).arg(spec.upstreamId));
	return uid;
}

void DecoderManager::removeDecoder(const QString &uid)
{
	for(int i = 0; i < m_decoders.size(); ++i) {
		if(m_decoders[i].uid != uid) continue;
		DecoderInstance d = m_decoders.takeAt(i);
		if(m_engine && d.proc) m_engine->removeProcessor(d.proc);
		for(const scopy::acq::DataKey &k : d.outKeys) {
			if(m_overlay) m_overlay->unregisterDecoder(k);
			if(m_store)   m_store->remove(k);
		}
		for(const QPointer<scopy::PlotAxisHandle> &h : d.handles) {
			if(h.isNull()) continue;
			if(!m_digitalMgr.isNull()) {
				m_digitalMgr->unregisterAnnotationBand(h);
			} else {
				if(m_plot) m_plot->removePlotAxisHandle(h);
				h->deleteLater();
			}
		}
		if(d.proc) d.proc->deleteLater();
		Q_EMIT decoderRemoved(uid);
		if(m_logger) m_logger->info(kMgrId, QStringLiteral("removed decoder ") + uid);
		return;
	}
}

int DecoderManager::pushStage(const QString &uid, const QString &decoderId)
{
	DecoderInstance *d = find(uid);
	if(!d || !d->proc) {
		if(m_logger)
			m_logger->warning(kMgrId, QStringLiteral("pushStage: unknown uid ") + uid);
		return -1;
	}
	if(isEngineRunning()) {
		if(m_logger)
			m_logger->warning(kMgrId,
				QStringLiteral("pushStage: refusing mid-run mutation for ") + uid);
		return -1;
	}
	if(!m_overlay || !m_plot) return -1;
	if(decoderId.isEmpty())   return -1;

	const int stageIndex = d->stageIds.size();

	d->stageIds.append(decoderId);
	scopy::decoder::DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	d->cfg.stack.push_back(stage);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder",
			QStringLiteral("%1/%2").arg(uid).arg(stageIndex),
			"annotations");
	d->outKeys.append(outKey);

	// popStagesFrom() truncates d->handles; always allocate fresh here.
	scopy::PlotAxisHandle *handle = nullptr;
	if(!m_digitalMgr.isNull()) {
		handle = m_digitalMgr->registerAnnotationBand(
			QStringLiteral("%1/%2").arg(uid).arg(stageIndex));
	} else {
		handle = attachHandle(nextBandPos());
	}
	d->handles.append(QPointer<scopy::PlotAxisHandle>(handle));

	d->proc->setConfig(d->cfg);
	d->proc->setOutputKeys(d->outKeys);
	m_overlay->registerDecoder(d->proc, outKey, handle,
	                           stageTitle(uid, decoderId, stageIndex));

	if(m_logger)
		m_logger->info(kMgrId, QStringLiteral("pushStage: %1 += %2 (index %3)")
			.arg(uid, decoderId).arg(stageIndex));
	return stageIndex;
}

void DecoderManager::popStagesFrom(const QString &uid, int fromIndex)
{
	DecoderInstance *d = find(uid);
	if(!d || !d->proc) return;
	if(isEngineRunning()) {
		if(m_logger)
			m_logger->warning(kMgrId,
				QStringLiteral("popStagesFrom: refusing mid-run mutation for ") + uid);
		return;
	}
	if(fromIndex < 1) fromIndex = 1; // never drop root
	if(fromIndex >= d->stageIds.size()) return;

	// Tear down popped stages: overlay curve, store key, cfg entry, handle.
	while(d->stageIds.size() > fromIndex) {
		const int idx = d->stageIds.size() - 1;
		const scopy::acq::DataKey k = d->outKeys[idx];
		if(m_overlay) m_overlay->unregisterDecoder(k);
		if(m_store)   m_store->remove(k);
		if(idx < d->handles.size()) {
			QPointer<scopy::PlotAxisHandle> h = d->handles[idx];
			if(!h.isNull()) {
				if(!m_digitalMgr.isNull()) {
					m_digitalMgr->unregisterAnnotationBand(h);
				} else {
					if(m_plot) m_plot->removePlotAxisHandle(h);
					h->deleteLater();
				}
			}
			d->handles.removeAt(idx);
		}
		d->outKeys.removeAt(idx);
		d->stageIds.removeAt(idx);
		d->cfg.stack.pop_back();
	}

	d->proc->setConfig(d->cfg);
	d->proc->setOutputKeys(d->outKeys);
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
	if(static_cast<int>(cfg.stack.size()) != d->stageIds.size()) {
		if(m_logger)
			m_logger->warning(kMgrId,
				QStringLiteral("applyConfig: stack size mismatch: cfg=%1 instance=%2")
					.arg(cfg.stack.size()).arg(d->stageIds.size()));
		return;
	}

	d->cfg = cfg;
	d->proc->setConfig(cfg);

	if(cfg.rootInput == scopy::decoder::RootInput::Annotations) {
		// Annotation-in: ignore orderedRawKeys; resolve source outKey.
		const QString srcUid = QString::fromStdString(cfg.annotationInput.sourceUid);
		DecoderInstance *src = find(srcUid);
		if(!src || cfg.annotationInput.sourceStageIndex < 0
		   || cfg.annotationInput.sourceStageIndex >= src->outKeys.size()) {
			if(m_logger) m_logger->warning(kMgrId,
				QStringLiteral("applyConfig: bad annotationInput (uid=%1 stage=%2)")
					.arg(srcUid).arg(cfg.annotationInput.sourceStageIndex));
			return;
		}
		const scopy::acq::DataKey srcKey = src->outKeys[cfg.annotationInput.sourceStageIndex];
		d->orderedRawKeys.clear();
		d->proc->setAnnotationInputKey(srcKey);
		d->proc->setWatchedKeys({srcKey});
	} else {
		d->orderedRawKeys = orderedRawKeys;
		d->proc->setOrderedRawKeys(orderedRawKeys);
		d->proc->setWatchedKeys(orderedRawKeys);

		if(m_store && m_windowSize > 0) {
			// Ensure history depth covers a full decoder window.
			const std::size_t depth = scopy::acq::DataStore::requiredHistoryDepth(
				static_cast<std::size_t>(m_windowSize),
				m_engine ? m_engine->bufferSize() : 1);
			for(const scopy::acq::DataKey &k : orderedRawKeys)
				m_store->ensureHistoryDepth(k, depth);
		}
	}

	if(m_logger)
		m_logger->info(kMgrId,
			QStringLiteral("applied config to %1 channels=%2 sampleRate=%3")
				.arg(uid).arg(orderedRawKeys.size()).arg(cfg.sampleRate));
}

void DecoderManager::setDecoderWindowSize(int n)
{
	m_windowSize = n;
	for(const DecoderInstance &d : m_decoders)
		if(d.proc) d.proc->setWindowSize(n);
}

} // namespace adc
} // namespace scopy
