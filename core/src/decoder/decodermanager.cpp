/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <core/decoder/decodermanager.h>

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/externaldecoderprocessor.h>
#include <core/decoder/decoderlogger.h>
#include <core/decoder/idecoderbackendfactory.h>

namespace scopy {
namespace decoder {

static constexpr const char *kMgrId = "decoder-manager";

DecoderManager::DecoderManager(scopy::acq::AcquisitionEngine *engine, scopy::acq::DataStore *store,
			       IDecoderBackendFactory *backendFactory, QObject *parent)
	: QObject(parent)
	, m_engine(engine)
	, m_store(store)
	, m_backendFactory(backendFactory)
{}

DecoderManager::~DecoderManager()
{
	// Every pointer here is normally already null at this point — see m_engine in the header.
	// This loop is the orderly path, removeDecoder() while the instrument is alive; a shutdown
	// walks it finding nothing to do. It must not assume otherwise: a raw d.proc->deleteLater()
	// segfaulted on every close.
	for(const DecoderInstance &d : m_decoders) {
		if(!m_engine.isNull() && !d.proc.isNull())
			m_engine->removeProcessor(d.proc);
		if(!d.proc.isNull())
			d.proc->deleteLater();
	}
	m_decoders.clear();
}

bool DecoderManager::isEngineRunning() const { return m_engine && m_engine->isRunning(); }

DecoderInstance *DecoderManager::find(const QString &uid)
{
	for(DecoderInstance &d : m_decoders)
		if(d.uid == uid)
			return &d;
	return nullptr;
}

QString DecoderManager::addDecoder(const QString &decoderId)
{
	if(!m_engine) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: no engine"));
		return {};
	}
	if(!m_backendFactory) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: no backend factory injected"));
		return {};
	}
	if(decoderId.isEmpty())
		return {};

	const QString uid = QString("%1-%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoder: backend factory returned null"));
		return {};
	}
	auto *proc = new scopy::acq::ExternalDecoderProcessor(uid, std::move(backend), m_engine);

	DecoderConfig cfg;
	cfg.sampleRate = 1.0e6;
	cfg.numChannels = 0;
	DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	cfg.stack.push_back(stage);
	proc->setConfig(cfg);
	proc->setWindowSize(m_windowSize);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder", uid + QStringLiteral("/0"), "annotations");
	proc->setOutputKeys({outKey});

	m_engine->addProcessor(proc);

	DecoderInstance d;
	d.uid = uid;
	d.stageIds = {decoderId};
	d.cfg = cfg;
	d.proc = proc;
	d.outKeys = {outKey};
	m_decoders.append(d);

	Q_EMIT decoderAdded(uid);
	Q_EMIT outKeyAdded(outKey);
	if(m_logger)
		m_logger->info(kMgrId, QStringLiteral("added decoder ") + uid);
	return uid;
}

QString DecoderManager::addDecoderFromAnnotations(const QString &decoderId, const AnnotationChainSpec &spec)
{
	if(!m_engine) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoderFromAnnotations: no engine"));
		return {};
	}
	if(!m_backendFactory) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoderFromAnnotations: no backend factory"));
		return {};
	}
	if(decoderId.isEmpty() || spec.upstreamId.isEmpty())
		return {};

	DecoderInstance *src = find(spec.sourceUid);
	if(!src) {
		if(m_logger)
			m_logger->warning(
				kMgrId,
				QStringLiteral("addDecoderFromAnnotations: unknown sourceUid %1").arg(spec.sourceUid));
		return {};
	}
	if(spec.sourceStageIndex < 0 || spec.sourceStageIndex >= src->outKeys.size()) {
		if(m_logger)
			m_logger->warning(
				kMgrId,
				QStringLiteral("addDecoderFromAnnotations: bad stage index %1 (source has %2 stages)")
					.arg(spec.sourceStageIndex)
					.arg(src->outKeys.size()));
		return {};
	}

	DecoderConfig cfg;
	cfg.sampleRate = src->cfg.sampleRate;
	cfg.numChannels = 0;
	DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	cfg.stack.push_back(stage);
	cfg.rootInput = RootInput::Annotations;
	cfg.annotationInput.sourceUid = spec.sourceUid.toStdString();
	cfg.annotationInput.sourceStageIndex = spec.sourceStageIndex;

	cfg.meta["annIn.upstreamId"] = spec.upstreamId.toStdString();
	for(auto it = spec.options.constBegin(); it != spec.options.constEnd(); ++it) {
		cfg.meta[std::string("annIn.") + it.key().toStdString()] = it.value().toStdString();
	}

	// Pre-check backend support before allocating.
	auto probeBackend = m_backendFactory->create();
	if(!probeBackend) {
		if(m_logger)
			m_logger->critical(kMgrId, QStringLiteral("addDecoderFromAnnotations: probe backend null"));
		return {};
	}
	if(!probeBackend->acceptsAnnotationInput(cfg)) {
		if(m_logger)
			m_logger->warning(
				kMgrId,
				QStringLiteral(
					"addDecoderFromAnnotations: backend rejects (upstream=%1, downstream=%2)")
					.arg(spec.upstreamId, decoderId));
		return {};
	}
	probeBackend.reset();

	const QString uid = QString("%1-%2").arg(decoderId).arg(m_uidCounter++);

	auto backend = m_backendFactory->create();
	if(!backend)
		return {};
	auto *proc = new scopy::acq::ExternalDecoderProcessor(uid, std::move(backend), m_engine);
	proc->setConfig(cfg);
	proc->setWindowSize(m_windowSize);

	const scopy::acq::DataKey outKey =
		scopy::acq::DataKey::withStage("decoder", uid + QStringLiteral("/0"), "annotations");
	proc->setOutputKeys({outKey});

	// Watch the source outKey; engine schedules on writes to watched keys.
	const scopy::acq::DataKey srcKey = src->outKeys[spec.sourceStageIndex];
	proc->setAnnotationInputKey(srcKey);
	proc->setWatchedKeys({srcKey});

	m_engine->addProcessor(proc);

	DecoderInstance d;
	d.uid = uid;
	d.stageIds = {decoderId};
	d.cfg = cfg;
	d.proc = proc;
	d.outKeys = {outKey};
	m_decoders.append(d);

	Q_EMIT decoderAdded(uid);
	Q_EMIT outKeyAdded(outKey);
	if(m_logger)
		m_logger->info(kMgrId,
			       QStringLiteral("added annotation-chained decoder %1 (source=%2 stage=%3 upstream=%4)")
				       .arg(uid, spec.sourceUid)
				       .arg(spec.sourceStageIndex)
				       .arg(spec.upstreamId));
	return uid;
}

void DecoderManager::removeDecoder(const QString &uid)
{
	for(int i = 0; i < m_decoders.size(); ++i) {
		if(m_decoders[i].uid != uid)
			continue;
		DecoderInstance d = m_decoders.takeAt(i);
		if(m_engine && d.proc)
			m_engine->removeProcessor(d.proc);
		if(m_store)
			m_store->releaseClaimant(uid);
		for(const scopy::acq::DataKey &k : d.outKeys) {
			if(m_store)
				m_store->remove(k);
		}
		if(d.proc)
			d.proc->deleteLater();
		if(!d.outKeys.isEmpty())
			Q_EMIT stagesRemoved(uid, d.outKeys);
		Q_EMIT decoderRemoved(uid);
		if(m_logger)
			m_logger->info(kMgrId, QStringLiteral("removed decoder ") + uid);
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
			m_logger->warning(kMgrId, QStringLiteral("pushStage: refusing mid-run mutation for ") + uid);
		return -1;
	}
	if(decoderId.isEmpty())
		return -1;

	const int stageIndex = d->stageIds.size();

	d->stageIds.append(decoderId);
	DecoderStage stage;
	stage.decoderId = decoderId.toStdString();
	d->cfg.stack.push_back(stage);

	const scopy::acq::DataKey outKey = scopy::acq::DataKey::withStage(
		"decoder", QStringLiteral("%1/%2").arg(uid).arg(stageIndex), "annotations");
	d->outKeys.append(outKey);

	d->proc->setConfig(d->cfg);
	d->proc->setOutputKeys(d->outKeys);

	if(m_logger)
		m_logger->info(kMgrId,
			       QStringLiteral("pushStage: %1 += %2 (index %3)").arg(uid, decoderId).arg(stageIndex));

	// Last, so a handler that reads find(uid) sees the finished stage.
	Q_EMIT stageAdded(uid, stageIndex);
	Q_EMIT outKeyAdded(outKey);
	return stageIndex;
}

void DecoderManager::popStagesFrom(const QString &uid, int fromIndex)
{
	DecoderInstance *d = find(uid);
	if(!d || !d->proc)
		return;
	if(isEngineRunning()) {
		if(m_logger)
			m_logger->warning(kMgrId,
					  QStringLiteral("popStagesFrom: refusing mid-run mutation for ") + uid);
		return;
	}
	if(fromIndex < 1)
		fromIndex = 1; // never drop root
	if(fromIndex >= d->stageIds.size())
		return;

	// Tear down popped stages: store key, cfg entry.
	QList<scopy::acq::DataKey> dropped;
	while(d->stageIds.size() > fromIndex) {
		const int idx = d->stageIds.size() - 1;
		const scopy::acq::DataKey k = d->outKeys[idx];
		dropped.append(k);
		if(m_store)
			m_store->remove(k);
		d->outKeys.removeAt(idx);
		d->stageIds.removeAt(idx);
		d->cfg.stack.pop_back();
	}

	d->proc->setConfig(d->cfg);
	d->proc->setOutputKeys(d->outKeys);

	if(!dropped.isEmpty())
		Q_EMIT stagesRemoved(uid, dropped);
}

void DecoderManager::applyConfig(const QString &uid, const DecoderConfig &cfg,
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
			m_logger->warning(kMgrId, QStringLiteral("applyConfig: refusing mid-run mutation for ") + uid);
		return;
	}
	if(static_cast<int>(cfg.stack.size()) != d->stageIds.size()) {
		if(m_logger)
			m_logger->warning(kMgrId,
					  QStringLiteral("applyConfig: stack size mismatch: cfg=%1 instance=%2")
						  .arg(cfg.stack.size())
						  .arg(d->stageIds.size()));
		return;
	}

	d->cfg = cfg;
	d->proc->setConfig(cfg);

	if(cfg.rootInput == RootInput::Annotations) {
		// Annotation-in: ignore orderedRawKeys; resolve source outKey.
		const QString srcUid = QString::fromStdString(cfg.annotationInput.sourceUid);
		DecoderInstance *src = find(srcUid);
		if(!src || cfg.annotationInput.sourceStageIndex < 0 ||
		   cfg.annotationInput.sourceStageIndex >= src->outKeys.size()) {
			if(m_logger)
				m_logger->warning(kMgrId,
						  QStringLiteral("applyConfig: bad annotationInput (uid=%1 stage=%2)")
							  .arg(srcUid)
							  .arg(cfg.annotationInput.sourceStageIndex));
			return;
		}
		const scopy::acq::DataKey srcKey = src->outKeys[cfg.annotationInput.sourceStageIndex];
		d->orderedRawKeys.clear();
		d->proc->setAnnotationInputKey(srcKey);
		d->proc->setWatchedKeys({srcKey});
		if(m_store)
			m_store->releaseClaimant(uid);
	} else {
		d->orderedRawKeys = orderedRawKeys;
		d->proc->setOrderedRawKeys(orderedRawKeys);
		d->proc->setWatchedKeys(orderedRawKeys);
		claimWindowDepth(*d);
	}

	if(m_logger)
		m_logger->info(kMgrId,
			       QStringLiteral("applied config to %1 channels=%2 sampleRate=%3")
				       .arg(uid)
				       .arg(orderedRawKeys.size())
				       .arg(cfg.sampleRate));
}

// Claim enough history on each raw input for one full decoder window. Claims
// are keyed by decoder uid, so re-claiming replaces the previous request and
// removeDecoder() releases it — the DataStore keeps the max across claimants.
void DecoderManager::claimWindowDepth(const DecoderInstance &d)
{
	if(!m_store)
		return;
	if(m_windowSize <= 0) {
		m_store->releaseClaimant(d.uid);
		return;
	}
	// In samples, which is the unit a decoder window is actually in. The store
	// converts to chunks against the length chunks are arriving in and re-derives it
	// on every push, so a resized acquisition buffer needs nothing from here — no
	// engine read, and no re-claim on a buffer size change.
	for(const scopy::acq::DataKey &k : d.orderedRawKeys)
		m_store->claimDepth(k, d.uid,
				    scopy::acq::DataStore::Claim::samples(static_cast<std::size_t>(m_windowSize)));
}

void DecoderManager::setDecoderWindowSize(int n)
{
	m_windowSize = n;
	for(const DecoderInstance &d : m_decoders) {
		if(d.proc)
			d.proc->setWindowSize(n);
		claimWindowDepth(d);
	}
}

} // namespace decoder
} // namespace scopy

#include "moc_decodermanager.cpp"
