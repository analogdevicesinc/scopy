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

#pragma once

#include "scopy-core_export.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

#include <core/acq_engine/datakey.h>
#include <core/decoder/idecoderbackend.h>

namespace scopy {
namespace acq {
class AcquisitionEngine;
class DataStore;
class ExternalDecoderProcessor;
} // namespace acq

namespace decoder {
class IDecoderBackendFactory;
class DecoderLogger;

// Runtime handle for one active decoder stack. cfg.stack[0] is the root.
// Processor is owned by the AcquisitionEngine.
struct DecoderInstance
{
	QString uid; // "<rootId>#<n>"
	QStringList stageIds;
	DecoderConfig cfg;
	QList<scopy::acq::DataKey> orderedRawKeys; // root bitIndex -> key
	// QPointer: parented to the engine, so it dies with it — before this manager. See
	// m_engine below.
	QPointer<scopy::acq::ExternalDecoderProcessor> proc;
	QList<scopy::acq::DataKey> outKeys; // one per stage
};

// Owns active decoder instances and wires their processors. Each processor writes its
// annotations to the DataStore; drawing is up to the view.
// Main-thread only; config mutation requires the engine to be stopped.
class SCOPY_CORE_EXPORT DecoderManager : public QObject
{
	Q_OBJECT
public:
	// backendFactory is non-owning and must outlive the manager.
	DecoderManager(scopy::acq::AcquisitionEngine *engine, scopy::acq::DataStore *store,
		       IDecoderBackendFactory *backendFactory, QObject *parent = nullptr);
	~DecoderManager() override;

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	// Returns the new uid or "" on failure.
	QString addDecoder(const QString &decoderId);

	// options are written into cfg.meta with the "annIn." prefix.
	struct AnnotationChainSpec
	{
		QString sourceUid;
		int sourceStageIndex{0};
		QString upstreamId;
		QMap<QString, QString> options;
	};

	// Root reads annotations from another instance; backend must
	// acceptsAnnotationInput(cfg). Returns new uid or "".
	QString addDecoderFromAnnotations(const QString &decoderId, const AnnotationChainSpec &spec);

	// Safe while engine is running.
	void removeDecoder(const QString &uid);

	// Returns the new stage index (>=1) or -1. Engine must be stopped.
	int pushStage(const QString &uid, const QString &decoderId);

	// Remove stages index >= fromIndex (fromIndex >= 1). Engine must be stopped.
	void popStagesFrom(const QString &uid, int fromIndex);

	// Engine must be stopped.
	void applyConfig(const QString &uid, const DecoderConfig &cfg,
			 const QList<scopy::acq::DataKey> &orderedRawKeys);

	// Propagates to every existing and future ExternalDecoderProcessor.
	void setDecoderWindowSize(int n);

	const QList<DecoderInstance> &decoders() const { return m_decoders; }
	DecoderInstance *find(const QString &uid);
	bool isEngineRunning() const;

Q_SIGNALS:
	void decoderAdded(const QString &uid);
	void decoderRemoved(const QString &uid);

	// A new output key on a decoder that already exists, so decoderAdded does not cover it
	// and neither does the engine: it only sees setOutputKeys() on a processor it already
	// holds, so blocksChanged never fires.
	void stageAdded(const QString &uid, int stageIndex);

	// A new output key, whether it came with a new decoder or with a stage pushed onto one
	// that already exists. decoderAdded and stageAdded name a uid and an index that a handler
	// then has to resolve back to this; emitting the key is that resolution done once, here,
	// where it is already in hand. Fires alongside them, never instead of them.
	void outKeyAdded(const scopy::acq::DataKey &key);

	// Output keys that no longer exist — popped stages, or every stage of a removed decoder.
	// The keys travel in the signal because by the time it fires they are already off the
	// instance, which for removeDecoder is gone.
	void stagesRemoved(const QString &uid, const QList<scopy::acq::DataKey> &keys);

private:
	void claimWindowDepth(const DecoderInstance &d);

	// QPointer, not raw: both are children of the instrument widget, which the plugin deletes
	// in deleteInstrument(), while this manager is a child of the controller and destroyed
	// later still. By the time ~DecoderManager runs they are already gone, and reading either
	// raw was a use-after-free on every shutdown.
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	QPointer<scopy::acq::DataStore> m_store;
	IDecoderBackendFactory *m_backendFactory{nullptr};

	QList<DecoderInstance> m_decoders;

	int m_uidCounter{0};

	// Samples/cycle for every ExternalDecoderProcessor; 0 = single-chunk.
	int m_windowSize{0};

	DecoderLogger *m_logger{nullptr};
};

} // namespace decoder
} // namespace scopy
