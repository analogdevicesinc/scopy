#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

#include <core/acq_engine/DataKey.h>
#include <core/decoder/IDecoderBackend.h>

namespace scopy {

class PlotAxis;
class PlotAxisHandle;
class PlotWidget;

namespace decoder {
class IDecoderBackendFactory;
class DecoderLogger;
} // namespace decoder

namespace acq {
class AcquisitionEngine;
class DataStore;
class ExternalDecoderProcessor;
} // namespace acq

namespace adc {

class DecoderOverlay;
class DigitalTrackManager;

// Runtime handle for one active decoder stack. cfg.stack[0] is the root.
// Processor is owned by the AcquisitionEngine.
struct DecoderInstance
{
	QString                                uid;             // "<rootId>#<n>"
	QStringList                            stageIds;
	scopy::decoder::DecoderConfig          cfg;
	QList<scopy::acq::DataKey>             orderedRawKeys;  // root bitIndex -> key
	// QPointer: parented to the engine, so it dies with it — before this manager. See
	// m_engine below.
	QPointer<scopy::acq::ExternalDecoderProcessor> proc;
	QList<scopy::acq::DataKey>             outKeys;         // one per stage
	QList<QPointer<scopy::PlotAxisHandle>> handles;         // one per stage; top of band
};

// Owns active decoder instances; wires processors and annotation-band axes.
// Main-thread only; config mutation requires the engine to be stopped.
//
// The plot and overlay are OPTIONAL. They serve one purpose — draggable annotation bands
// on a single PlotWidget — and nothing on the data path needs them: each processor writes
// its annotations to the DataStore, so a store-backed view draws without help from here.
// Unset, decoders still run and publish and are simply not drawn by this class, which is
// what the multi-plot acq instrument needs.
class DecoderManager : public QObject
{
	Q_OBJECT
public:
	// backendFactory is non-owning and must outlive the manager.
	DecoderManager(scopy::acq::AcquisitionEngine *engine,
	               scopy::acq::DataStore *store,
	               scopy::decoder::IDecoderBackendFactory *backendFactory,
	               QObject *parent = nullptr);
	~DecoderManager() override;

	// Both optional — see the class comment.
	void setPlot(scopy::PlotWidget *plot);
	void setOverlay(DecoderOverlay *overlay);
	void setLogger(scopy::decoder::DecoderLogger *lg) { m_logger = lg; }

	// Route handles onto a dedicated digital-track manager (mixed-signal view).
	void setDigitalTrackManager(DigitalTrackManager *mgr);

	// Returns the new uid or "" on failure.
	QString addDecoder(const QString &decoderId);

	// options are written into cfg.meta with the "annIn." prefix.
	struct AnnotationChainSpec
	{
		QString                  sourceUid;
		int                      sourceStageIndex{0};
		QString                  upstreamId;
		QMap<QString, QString>   options;
	};

	// Root reads annotations from another instance; backend must
	// acceptsAnnotationInput(cfg). Returns new uid or "".
	QString addDecoderFromAnnotations(const QString &decoderId,
	                                  const AnnotationChainSpec &spec);

	// Safe while engine is running.
	void removeDecoder(const QString &uid);

	// Returns the new stage index (>=1) or -1. Engine must be stopped.
	int pushStage(const QString &uid, const QString &decoderId);

	// Remove stages index >= fromIndex (fromIndex >= 1). Engine must be stopped.
	void popStagesFrom(const QString &uid, int fromIndex);

	// Engine must be stopped.
	void applyConfig(const QString &uid,
	                 const scopy::decoder::DecoderConfig &cfg,
	                 const QList<scopy::acq::DataKey> &orderedRawKeys);

	// Propagates to every existing and future ExternalDecoderProcessor.
	void setDecoderWindowSize(int n);

	const QList<DecoderInstance> &decoders() const { return m_decoders; }
	DecoderInstance              *find(const QString &uid);
	bool                          isEngineRunning() const;

Q_SIGNALS:
	void decoderAdded(const QString &uid);
	void decoderRemoved(const QString &uid);

	// A new output key on a decoder that already exists, so decoderAdded does not cover it
	// and neither does the engine: it only sees setOutputKeys() on a processor it already
	// holds, so blocksChanged never fires.
	void stageAdded(const QString &uid, int stageIndex);

	// Output keys that no longer exist — popped stages, or every stage of a removed decoder.
	// The keys travel in the signal because by the time it fires they are already off the
	// instance, which for removeDecoder is gone.
	void stagesRemoved(const QString &uid, const QList<scopy::acq::DataKey> &keys);

private:
	double nextBandPos();
	scopy::PlotAxisHandle *attachHandle(double initialPos);
	void claimWindowDepth(const DecoderInstance &d);

	// QPointer, not raw: both are children of the instrument widget, which the plugin deletes
	// in deleteInstrument(), while this manager is a child of the controller and destroyed
	// later still. By the time ~DecoderManager runs they are already gone, and reading either
	// raw was a use-after-free on every shutdown.
	QPointer<scopy::acq::AcquisitionEngine>  m_engine;
	QPointer<scopy::acq::DataStore>          m_store;
	scopy::decoder::IDecoderBackendFactory *m_backendFactory{nullptr};
	QPointer<scopy::PlotWidget>             m_plot;
	QPointer<DecoderOverlay>                m_overlay;
	QPointer<DigitalTrackManager>           m_digitalMgr;

	QList<DecoderInstance>          m_decoders;

	int                             m_uidCounter{0};

	// Vertical stacking cursor (main y-axis scale coords) for band handles.
	double                          m_nextBandPos{0.0};
	bool                            m_bandCursorInit{false};

	// Samples/cycle for every ExternalDecoderProcessor; 0 = single-chunk.
	int                             m_windowSize{0};

	scopy::decoder::DecoderLogger  *m_logger{nullptr};
};

} // namespace adc
} // namespace scopy
