#include "DecoderManager.h"

#include "DecoderOverlay.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/ExternalDecoderProcessor.h>
#include <core/decoder/IDecoderBackendFactory.h>

#include <gui/plotaxis.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <QLoggingCategory>
#include <QPen>

Q_LOGGING_CATEGORY(CAT_DECODER_MGR, "DecoderManager")

namespace scopy {
namespace adc {

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
	return axis;
}

QString DecoderManager::addDecoder(const QString &decoderId)
{
	if(!m_engine) {
		qCWarning(CAT_DECODER_MGR) << "addDecoder: no engine";
		return {};
	}
	if(!m_overlay || !m_plot) {
		qCWarning(CAT_DECODER_MGR)
			<< "addDecoder: overlay/plot not set (call setPlot/setOverlay first)";
		return {};
	}
	if(!m_backendFactory) {
		qCWarning(CAT_DECODER_MGR) << "addDecoder: no backend factory injected";
		return {};
	}
	if(decoderId.isEmpty()) return {};

	const QString uid = QString("%1#%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend) {
		qCWarning(CAT_DECODER_MGR) << "addDecoder: backend factory returned null";
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
	qCInfo(CAT_DECODER_MGR) << "added decoder" << uid;
	return uid;
}

void DecoderManager::removeDecoder(const QString &uid)
{
	for(int i = 0; i < m_decoders.size(); ++i) {
		if(m_decoders[i].uid != uid) continue;
		DecoderInstance d = m_decoders.takeAt(i);
		if(m_engine && d.proc) m_engine->removeProcessor(d.proc);
		// NOTE: the axis and AnnotationCurve inside the overlay are
		// intentionally leaked at removal time — DecoderOverlay does
		// not expose an unregister API, and axes are children of the
		// plot with no per-decoder tracking here. For the demo flow
		// that's acceptable; a follow-up should add
		// DecoderOverlay::unregisterDecoder(outKey).
		if(d.proc) d.proc->deleteLater();
		Q_EMIT decoderRemoved(uid);
		qCInfo(CAT_DECODER_MGR) << "removed decoder" << uid;
		return;
	}
}

void DecoderManager::applyConfig(const QString &uid,
                                 const scopy::decoder::DecoderConfig &cfg,
                                 const QList<scopy::acq::DataKey> &orderedRawKeys)
{
	DecoderInstance *d = find(uid);
	if(!d || !d->proc) {
		qCWarning(CAT_DECODER_MGR) << "applyConfig: unknown uid" << uid;
		return;
	}
	if(isEngineRunning()) {
		qCWarning(CAT_DECODER_MGR)
			<< "applyConfig: refusing mid-run mutation for" << uid;
		return;
	}

	d->cfg            = cfg;
	d->orderedRawKeys = orderedRawKeys;
	d->proc->setConfig(cfg);
	d->proc->setOrderedRawKeys(orderedRawKeys);
	d->proc->setWatchedKeys(orderedRawKeys);
	qCInfo(CAT_DECODER_MGR)
		<< "applied config to" << uid
		<< "channels=" << orderedRawKeys.size()
		<< "sampleRate=" << cfg.sampleRate;
	Q_EMIT configApplied(uid);
}

} // namespace adc
} // namespace scopy
